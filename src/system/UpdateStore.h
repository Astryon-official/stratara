#pragma once

#include <QObject>
#include <QSettings>
#include <QCoreApplication>
#include <QMutex>

namespace Stratara::System {

struct PendingUpdate {
    QString versionTag;
    QString changelog;
    QString downloadUrl;
    bool isValid() const { return !versionTag.isEmpty() && !downloadUrl.isEmpty(); }
};

class UpdateStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 lastCheckedAtMillis READ lastCheckedAtMillis NOTIFY lastCheckedAtMillisChanged)
    Q_PROPERTY(bool hasPendingUpdate READ hasPendingUpdate NOTIFY pendingUpdateChanged)
    Q_PROPERTY(PendingUpdate pendingUpdate READ pendingUpdate NOTIFY pendingUpdateChanged)
    Q_PROPERTY(QString dismissedNoticeTag READ dismissedNoticeTag NOTIFY dismissedNoticeTagChanged)

public:
    explicit UpdateStore(QObject *parent = nullptr);
    ~UpdateStore() override = default;

    qint64 lastCheckedAtMillis() const;
    bool hasPendingUpdate() const;
    PendingUpdate pendingUpdate() const;
    QString dismissedNoticeTag() const;

    Q_INVOKABLE void markCheckedNow(qint64 nowMillis);
    Q_INVOKABLE void setPendingUpdate(const QString &versionTag, const QString &changelog, const QString &downloadUrl);
    Q_INVOKABLE void clearPendingUpdate();
    Q_INVOKABLE void dismissNotice(const QString &versionTag);

signals:
    void lastCheckedAtMillisChanged(qint64 millis);
    void pendingUpdateChanged();
    void dismissedNoticeTagChanged(const QString &tag);

private:
    QSettings *m_settings;
    mutable QMutex m_mutex;

    static constexpr const char *LAST_CHECKED_KEY = "last_checked_at_millis";
    static constexpr const char *PENDING_TAG_KEY = "pending_version_tag";
    static constexpr const char *PENDING_CHANGELOG_KEY = "pending_changelog";
    static constexpr const char *PENDING_URL_KEY = "pending_download_url";
    static constexpr const char *DISMISSED_NOTICE_TAG_KEY = "dismissed_notice_tag";
};

} // namespace Stratara::System

Q_DECLARE_METATYPE(Stratara::System::PendingUpdate)