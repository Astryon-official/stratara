#include "AppListCache.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace Stratara::System {

AppListCache::AppListCache(QObject *parent)
    : QObject(parent)
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QString cacheDir = configPath + "/stratara";
    QDir().mkpath(cacheDir);
    m_cacheFile = cacheDir + "/app_list.json";
}

QList<AppInfo> AppListCache::read()
{
    QMutexLocker locker(&m_mutex);

    QFile file(m_cacheFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isArray()) {
        qWarning() << "Failed to parse app list cache:" << error.errorString();
        return {};
    }

    QJsonArray arr = doc.array();
    QList<AppInfo> apps;
    apps.reserve(arr.size());

    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        AppInfo app;
        app.name = obj["label"].toString();
        app.packageName = obj["pkg"].toString();
        app.activityName = obj["activity"].toString();
        app.isTvApp = obj["tv"].toBool();
        apps.append(app);
    }

    m_lastWritten = apps;
    return apps;
}

void AppListCache::write(const QList<AppInfo> &apps)
{
    QMutexLocker locker(&m_mutex);

    // Manual comparison since Q_GADGET structs don't work well with QList::operator==
    if (apps.size() == m_lastWritten.size()) {
        bool equal = true;
        for (int i = 0; i < apps.size(); ++i) {
            if (!(apps[i] == m_lastWritten[i])) {
                equal = false;
                break;
            }
        }
        if (equal) {
            return;
        }
    }

    QJsonArray arr;
    for (const AppInfo &app : apps) {
        QJsonObject obj;
        obj["label"] = app.name;
        obj["pkg"] = app.packageName;
        obj["activity"] = app.activityName;
        obj["tv"] = app.isTvApp;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(m_cacheFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Failed to write app list cache:" << file.errorString();
        return;
    }

    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();
    m_lastWritten = apps;
}

} // namespace Stratara::System