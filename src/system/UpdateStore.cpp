#include "UpdateStore.h"

#include <QSettings>
#include <QCoreApplication>

namespace Stratara::System {

UpdateStore::UpdateStore(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope,
                               QCoreApplication::organizationName(),
                               QCoreApplication::applicationName() + "_updates", this))
{
}

qint64 UpdateStore::lastCheckedAtMillis() const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->value(LAST_CHECKED_KEY, 0LL).toLongLong();
}

bool UpdateStore::hasPendingUpdate() const
{
    QMutexLocker locker(&m_mutex);
    return !m_settings->value(PENDING_TAG_KEY).toString().isEmpty();
}

PendingUpdate UpdateStore::pendingUpdate() const
{
    QMutexLocker locker(&m_mutex);
    PendingUpdate update;
    update.versionTag = m_settings->value(PENDING_TAG_KEY).toString();
    update.changelog = m_settings->value(PENDING_CHANGELOG_KEY).toString();
    update.downloadUrl = m_settings->value(PENDING_URL_KEY).toString();
    return update;
}

QString UpdateStore::dismissedNoticeTag() const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->value(DISMISSED_NOTICE_TAG_KEY).toString();
}

void UpdateStore::markCheckedNow(qint64 nowMillis)
{
    QMutexLocker locker(&m_mutex);
    m_settings->setValue(LAST_CHECKED_KEY, nowMillis);
    m_settings->sync();
    emit lastCheckedAtMillisChanged(nowMillis);
}

void UpdateStore::setPendingUpdate(const QString &versionTag, const QString &changelog, const QString &downloadUrl)
{
    QMutexLocker locker(&m_mutex);
    m_settings->setValue(PENDING_TAG_KEY, versionTag);
    m_settings->setValue(PENDING_CHANGELOG_KEY, changelog);
    m_settings->setValue(PENDING_URL_KEY, downloadUrl);
    m_settings->sync();
    emit pendingUpdateChanged();
}

void UpdateStore::clearPendingUpdate()
{
    QMutexLocker locker(&m_mutex);
    m_settings->remove(PENDING_TAG_KEY);
    m_settings->remove(PENDING_CHANGELOG_KEY);
    m_settings->remove(PENDING_URL_KEY);
    m_settings->sync();
    emit pendingUpdateChanged();
}

void UpdateStore::dismissNotice(const QString &versionTag)
{
    QMutexLocker locker(&m_mutex);
    m_settings->setValue(DISMISSED_NOTICE_TAG_KEY, versionTag);
    m_settings->sync();
    emit dismissedNoticeTagChanged(versionTag);
}

} // namespace Stratara::System