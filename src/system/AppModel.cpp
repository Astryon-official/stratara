#include "AppModel.h"

#include <QDesktopServices>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QIcon>
#include <QDirIterator>

namespace Stratara::System {

AppModel::AppModel(Stratara::Core::Application *app, QObject *parent)
    : QAbstractListModel(parent)
    , m_app(app)
    , m_watcher(new QFileSystemWatcher(this))
    , m_refreshTimer(new QTimer(this))
{
    m_refreshTimer->setSingleShot(true);
    m_refreshTimer->setInterval(500);
    connect(m_refreshTimer, &QTimer::timeout, this, &AppModel::scanApplications);

    connect(m_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](const QString &) { m_refreshTimer->start(); });
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, [this](const QString &) { m_refreshTimer->start(); });

    // Initial scan
    QTimer::singleShot(0, this, &AppModel::refresh);
}

int AppModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_apps.count();
}

QVariant AppModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_apps.count()) {
        return QVariant();
    }

    const AppInfo &app = m_apps[index.row()];

    switch (role) {
    case NameRole:
        return app.name;
    case PackageNameRole:
        return app.packageName;
    case ExecCommandRole:
        return app.execCommand;
    case IconNameRole:
        return app.iconName;
    case CommentRole:
        return app.comment;
    case CategoriesRole:
        return app.categories;
    case IsTerminalRole:
        return app.isTerminal;
    case NoDisplayRole:
        return app.noDisplay;
    case HiddenRole:
        return app.hidden;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AppModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {PackageNameRole, "packageName"},
        {ExecCommandRole, "execCommand"},
        {IconNameRole, "iconName"},
        {CommentRole, "comment"},
        {CategoriesRole, "categories"},
        {IsTerminalRole, "isTerminal"},
        {NoDisplayRole, "noDisplay"},
        {HiddenRole, "hidden"}
    };
}

bool AppModel::isLoading() const
{
    return m_isLoading;
}

void AppModel::refresh()
{
    if (m_isLoading) {
        return;
    }

    m_isLoading = true;
    emit isLoadingChanged(true);

    // Clear watcher
    m_watcher->removePaths(m_watcher->directories());
    m_watcher->removePaths(m_watcher->files());

    // Scan in background
    QTimer::singleShot(0, this, &AppModel::scanApplications);
}

AppInfo AppModel::getApp(int index) const
{
    if (index >= 0 && index < m_apps.count()) {
        return m_apps[index];
    }
    return AppInfo();
}

int AppModel::findApp(const QString &packageName) const
{
    for (int i = 0; i < m_apps.count(); ++i) {
        if (m_apps[i].packageName == packageName) {
            return i;
        }
    }
    return -1;
}

bool AppModel::launchApp(const QString &packageName)
{
    int index = findApp(packageName);
    if (index < 0) {
        return false;
    }

    return launchAppWithOptions(packageName, QStringList());
}

bool AppModel::launchAppWithOptions(const QString &packageName, const QStringList &args)
{
    int index = findApp(packageName);
    if (index < 0) {
        return false;
    }

    const AppInfo &app = m_apps[index];

    // Use gtk-launch or kde-open for proper desktop integration
    // Fallback to direct exec
    QStringList cmdParts = app.execCommand.split(' ', Qt::SkipEmptyParts);
    if (cmdParts.isEmpty()) {
        return false;
    }

    QString program = cmdParts.takeFirst();
    QStringList fullArgs = cmdParts + args;

    QProcess *process = new QProcess(this);
    process->setProgram(program);
    process->setArguments(fullArgs);
    process->setProcessChannelMode(QProcess::ForwardedChannels);
    process->startDetached();

    return true;
}

void AppModel::scanApplications()
{
    beginResetModel();
    m_apps.clear();

    QStringList dirs = getApplicationDirs();
    for (const QString &dir : dirs) {
        m_watcher->addPath(dir);

        QDirIterator it(dir, QStringList() << "*.desktop", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            parseDesktopFile(it.filePath());
        }
    }

    // Sort by name
    std::sort(m_apps.begin(), m_apps.end(),
              [](const AppInfo &a, const AppInfo &b) {
                  return a.name.toLower() < b.name.toLower();
              });

    endResetModel();

    m_isLoading = false;
    emit isLoadingChanged(false);
    emit countChanged();
    emit appsRefreshed();

    if (!m_initialScanDone) {
        m_initialScanDone = true;
    }
}

void AppModel::parseDesktopFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    AppInfo app;
    app.packageName = QFileInfo(filePath).baseName();

    QTextStream in(&file);
    bool inDesktopEntry = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line == "[Desktop Entry]") {
            inDesktopEntry = true;
            continue;
        }

        if (line.startsWith('[') && line != "[Desktop Entry]") {
            inDesktopEntry = false;
            continue;
        }

        if (!inDesktopEntry) {
            continue;
        }

        if (line.startsWith('#') || !line.contains('=')) {
            continue;
        }

        int eqPos = line.indexOf('=');
        QString key = line.left(eqPos).trimmed();
        QString value = line.mid(eqPos + 1).trimmed();

        if (key == "Name") {
            app.name = value;
        } else if (key == "Exec") {
            // Remove field codes (%f, %u, %F, %U, etc.)
            app.execCommand = value;
            app.execCommand.remove(QRegularExpression("%[fFuUdDnNickvm]"));
            app.execCommand = app.execCommand.trimmed();
        } else if (key == "Icon") {
            app.iconName = value;
        } else if (key == "Comment") {
            app.comment = value;
        } else if (key == "Categories") {
            app.categories = value.split(';', Qt::SkipEmptyParts);
        } else if (key == "Terminal") {
            app.isTerminal = (value.toLower() == "true");
        } else if (key == "NoDisplay") {
            app.noDisplay = (value.toLower() == "true");
        } else if (key == "Hidden") {
            app.hidden = (value.toLower() == "true");
        } else if (key == "Type" && value != "Application") {
            // Not an application
            return;
        }
    }

    if (isValidApplication(app)) {
        m_apps.append(app);
    }
}

QString AppModel::findIcon(const QString &iconName) const
{
    if (iconName.isEmpty()) {
        return QString();
    }

    // If it's a full path, return as-is
    if (iconName.startsWith('/')) {
        return iconName;
    }

    // Search in standard icon directories
    QStringList iconDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    iconDirs.append("/usr/share/icons");
    iconDirs.append("/usr/local/share/icons");

    QStringList themes = {"hicolor", "breeze", "Adwaita", "oxygen"};

    for (const QString &baseDir : iconDirs) {
        for (const QString &theme : themes) {
            QString themeDir = baseDir + "/icons/" + theme;
            QDir dir(themeDir);
            if (!dir.exists()) {
                continue;
            }

            // Search in scalable, then various sizes
            QStringList subdirs = {"scalable/apps", "scalable", "64x64/apps", "64x64",
                                   "48x48/apps", "48x48", "32x32/apps", "32x32",
                                   "256x256/apps", "256x256", "128x128/apps", "128x128"};

            for (const QString &subdir : subdirs) {
                QString fullPath = themeDir + "/" + subdir + "/" + iconName;
                if (QFile::exists(fullPath + ".svg")) {
                    return fullPath + ".svg";
                }
                if (QFile::exists(fullPath + ".png")) {
                    return fullPath + ".png";
                }
                if (QFile::exists(fullPath + ".xpm")) {
                    return fullPath + ".xpm";
                }
            }
        }
    }

    // Try QIcon theme lookup
    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull()) {
        return iconName; // Return name for theme lookup
    }

    return QString();
}

QStringList AppModel::getApplicationDirs() const
{
    QStringList dirs;

    // System directories
    dirs << "/usr/share/applications";
    dirs << "/usr/local/share/applications";
    dirs << "/var/lib/flatpak/exports/share/applications";
    dirs << QString(qgetenv("HOME")) + "/.local/share/flatpak/exports/share/applications";
    dirs << QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);

    // User directories
    dirs << QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);

    // Remove duplicates and non-existent
    QStringList result;
    for (const QString &dir : dirs) {
        QDir d(dir);
        if (d.exists() && !result.contains(dir)) {
            result.append(dir);
        }
    }

    return result;
}

bool AppModel::isValidApplication(const AppInfo &app) const
{
    if (app.name.isEmpty() || app.execCommand.isEmpty()) {
        return false;
    }
    if (app.noDisplay || app.hidden) {
        return false;
    }
    if (app.isTerminal) {
        return false; // Skip terminal apps for TV interface
    }

    // Only show apps with relevant categories for TV/launcher
    QStringList relevantCategories = {
        "Game", "AudioVideo", "Audio", "Video", "Player", "TV",
        "Graphics", "Network", "WebBrowser", "Office", "Education",
        "Science", "Settings", "System", "Utility", "Accessibility"
    };

    // If no categories, include it (some apps don't declare categories)
    if (app.categories.isEmpty()) {
        return true;
    }

    for (const QString &cat : app.categories) {
        if (relevantCategories.contains(cat, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

} // namespace Stratara::System