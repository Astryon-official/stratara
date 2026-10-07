#include "UpdateChecker.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QCoreApplication>
#include <QDebug>

namespace Stratara::System {

UpdateChecker::UpdateChecker(UpdateStore *updateStore, QObject *parent)
    : QObject(parent)
    , m_updateStore(updateStore)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void UpdateChecker::checkForUpdate(qint64 nowMillis, bool force)
{
    QMutexLocker locker(&m_mutex);

    if (m_currentReply) {
        qWarning() << "Update check already in progress";
        return;
    }

    qint64 lastChecked = m_updateStore->lastCheckedAtMillis();
    if (!force && nowMillis - lastChecked < MIN_CHECK_INTERVAL_MILLIS) {
        // Return pending update if available, otherwise up-to-date
        PendingUpdate pending = m_updateStore->pendingUpdate();
        if (pending.isValid()) {
            emit checkFinished(UpdateResult::updateAvailable(pending.versionTag, pending.changelog, pending.downloadUrl));
        } else {
            emit checkFinished(UpdateResult::upToDate());
        }
        return;
    }

    m_checkTimestamp = nowMillis;
    m_forceCheck = force;
    fetchLatestRelease();
}

bool UpdateChecker::isNewerThanInstalled(const QString &versionTag) const
{
    return isNewer(versionTag, installedVersionName());
}

void UpdateChecker::fetchLatestRelease()
{
    QNetworkRequest request{QUrl(RELEASES_URL)};
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("User-Agent", "Stratara/0.1.0");

    m_currentReply = m_networkManager->get(request);

    connect(m_currentReply, &QNetworkReply::finished, this, [this]() {
        QMutexLocker locker(&m_mutex);
        handleReply();
    });

    // Timeout
    QTimer::singleShot(CONNECT_TIMEOUT_MILLIS, this, [this]() {
        QMutexLocker locker(&m_mutex);
        if (m_currentReply && m_currentReply->isRunning()) {
            m_currentReply->abort();
        }
    });
}

void UpdateChecker::handleReply()
{
    if (!m_currentReply) {
        return;
    }

    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    UpdateResult result;

    if (reply->error() != QNetworkReply::NoError) {
        result = UpdateResult::error(QString("Network error: %1").arg(reply->errorString()));
    } else {
        QByteArray data = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            result = UpdateResult::error("Invalid JSON response from GitHub");
        } else {
            QJsonObject json = doc.object();
            QString tag = json["tag_name"].toString();
            QString changelog = json["body"].toString();

            QJsonArray assets = json["assets"].toArray();
            QString downloadUrl;

            for (const QJsonValue &assetVal : assets) {
                QJsonObject asset = assetVal.toObject();
                QString name = asset["name"].toString();
                if (name.endsWith(".AppImage") || name.endsWith(".flatpak") ||
                    name.endsWith(".deb") || name.endsWith(".rpm") ||
                    name.endsWith(".tar.gz") || name.endsWith(".tar.xz")) {
                    downloadUrl = asset["browser_download_url"].toString();
                    break;
                }
            }

            if (downloadUrl.isEmpty()) {
                result = UpdateResult::error("No suitable release asset found");
            } else if (!isTrustedDownloadUrl(downloadUrl)) {
                result = UpdateResult::error("Release asset from untrusted host");
            } else {
                m_updateStore->markCheckedNow(m_checkTimestamp);

                QString currentVersion = installedVersionName();
                if (isNewer(tag, currentVersion)) {
                    result = UpdateResult::updateAvailable(tag, changelog, downloadUrl);
                } else {
                    result = UpdateResult::upToDate();
                }
            }
        }
    }

    reply->deleteLater();
    emit checkFinished(result);
}

bool UpdateChecker::isTrustedDownloadUrl(const QString &url) const
{
    QUrl parsedUrl(url);
    if (parsedUrl.scheme() != "https") {
        return false;
    }

    QString host = parsedUrl.host();
    return host == "github.com" ||
           host == "objects.githubusercontent.com" ||
           host.endsWith(".github.com");
}

bool UpdateChecker::isNewer(const QString &latestTag, const QString &currentVersion) const
{
    QString latest = latestTag;
    if (latest.startsWith('v')) latest = latest.mid(1);
    QString current = currentVersion;
    if (current.startsWith('v')) current = current.mid(1);

    QStringList latestParts = latest.split('.');
    QStringList currentParts = current.split('.');

    int maxParts = qMax(latestParts.size(), currentParts.size());
    for (int i = 0; i < maxParts; ++i) {
        int l = (i < latestParts.size()) ? latestParts[i].toInt() : 0;
        int c = (i < currentParts.size()) ? currentParts[i].toInt() : 0;
        if (l != c) {
            return l > c;
        }
    }
    return false;
}

QString UpdateChecker::installedVersionName() const
{
    return QCoreApplication::applicationVersion();
}

} // namespace Stratara::System