#pragma once

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QMutex>

#include "AppModel.h"

namespace Stratara::System {

class AppListCache : public QObject
{
    Q_OBJECT

public:
    explicit AppListCache(QObject *parent = nullptr);
    ~AppListCache() override = default;

    Q_INVOKABLE QList<AppInfo> read();
    Q_INVOKABLE void write(const QList<AppInfo> &apps);

private:
    QString m_cacheFile;
    QList<AppInfo> m_lastWritten;
    mutable QMutex m_mutex;
};

} // namespace Stratara::System