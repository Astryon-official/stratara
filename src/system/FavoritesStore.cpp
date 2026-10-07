#include "FavoritesStore.h"

#include <QSettings>
#include <QCoreApplication>

namespace Stratara::System {

FavoritesStore::FavoritesStore(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope,
                               QCoreApplication::organizationName(),
                               QCoreApplication::applicationName() + "_favorites", this))
{
}

QStringList FavoritesStore::favorites() const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->value(FAVORITES_KEY, QStringList()).toStringList();
}

bool FavoritesStore::seeded() const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->value(SEEDED_KEY, false).toBool();
}

void FavoritesStore::setFavorites(const QStringList &packages)
{
    QMutexLocker locker(&m_mutex);
    m_settings->setValue(FAVORITES_KEY, packages);
    m_settings->sync();
    emit favoritesChanged(packages);
}

void FavoritesStore::toggle(const QString &packageName)
{
    QMutexLocker locker(&m_mutex);
    QStringList current = m_settings->value(FAVORITES_KEY, QStringList()).toStringList();
    if (!current.removeAll(packageName)) {
        current.append(packageName);
    }
    m_settings->setValue(FAVORITES_KEY, current);
    m_settings->sync();
    emit favoritesChanged(current);
}

void FavoritesStore::markSeeded()
{
    QMutexLocker locker(&m_mutex);
    m_settings->setValue(SEEDED_KEY, true);
    m_settings->sync();
    emit seededChanged(true);
}

} // namespace Stratara::System