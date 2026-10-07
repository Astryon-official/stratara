#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QQmlEngine>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QProcess>
#include <QStandardPaths>

#include "core/Application.h"

namespace Stratara::System {

struct AppInfo {
    QString name;
    QString packageName;  // desktop file ID
    QString execCommand;
    QString iconName;
    QString comment;
    QStringList categories;
    bool isTerminal = false;
    bool noDisplay = false;
    bool hidden = false;
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString packageName MEMBER packageName)
    Q_PROPERTY(QString execCommand MEMBER execCommand)
    Q_PROPERTY(QString iconName MEMBER iconName)
    Q_PROPERTY(QString comment MEMBER comment)
    Q_PROPERTY(QStringList categories MEMBER categories)
    Q_PROPERTY(bool isTerminal MEMBER isTerminal)
    Q_PROPERTY(bool noDisplay MEMBER noDisplay)
    Q_PROPERTY(bool hidden MEMBER hidden)
};

class AppModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PackageNameRole,
        ExecCommandRole,
        IconNameRole,
        CommentRole,
        CategoriesRole,
        IsTerminalRole,
        NoDisplayRole,
        HiddenRole
    };
    Q_ENUM(Roles)

    explicit AppModel(Stratara::Core::Application *app, QObject *parent = nullptr);
    ~AppModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isLoading() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE AppInfo getApp(int index) const;
    Q_INVOKABLE int findApp(const QString &packageName) const;
    Q_INVOKABLE bool launchApp(const QString &packageName);
    Q_INVOKABLE bool launchAppWithOptions(const QString &packageName, const QStringList &args);

signals:
    void countChanged();
    void isLoadingChanged(bool loading);
    void appsRefreshed();

private:
    void scanApplications();
    void parseDesktopFile(const QString &filePath);
    QString findIcon(const QString &iconName) const;
    QStringList getApplicationDirs() const;
    bool isValidApplication(const AppInfo &app) const;

    Stratara::Core::Application *m_app;
    QList<AppInfo> m_apps;
    QFileSystemWatcher *m_watcher;
    QTimer *m_refreshTimer;
    bool m_isLoading = false;
    bool m_initialScanDone = false;
};

} // namespace Stratara::System

Q_DECLARE_METATYPE(Stratara::System::AppInfo)