#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QTimer>
#include <QCoreApplication>
#include <QMutex>

#include "UpdateResult.h"
#include "UpdateStore.h"

namespace Stratara::System {

class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(UpdateStore *updateStore, QObject *parent = nullptr);
    ~UpdateChecker() override = default;

    Q_INVOKABLE void checkForUpdate(qint64 nowMillis, bool force = false);
    Q_INVOKABLE bool isNewerThanInstalled(const QString &versionTag) const;

    Q_INVOKABLE void setReleasesUrl(const QString &url);

signals:
    void checkFinished(const UpdateResult &result);

private:
    void fetchLatestRelease();
    void handleReply();
    bool isTrustedDownloadUrl(const QString &url) const;
    bool isNewer(const QString &latestTag, const QString &currentVersion) const;
    QString installedVersionName() const;

    UpdateStore *m_updateStore;
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply = nullptr;
    qint64 m_checkTimestamp = 0;
    bool m_forceCheck = false;
    mutable QMutex m_mutex;
    QString m_releasesUrl;

    static constexpr qint64 MIN_CHECK_INTERVAL_MILLIS = 5 * 60 * 1000; // 5 minutes
    static constexpr int CONNECT_TIMEOUT_MILLIS = 10000;
    static constexpr int READ_TIMEOUT_MILLIS = 10000;
};

} // namespace Stratara::System