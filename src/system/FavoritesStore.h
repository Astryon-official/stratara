#pragma once

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QMutex>

namespace Stratara::System {

class FavoritesStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList favorites READ favorites NOTIFY favoritesChanged)
    Q_PROPERTY(bool seeded READ seeded NOTIFY seededChanged)

public:
    explicit FavoritesStore(QObject *parent = nullptr);
    ~FavoritesStore() override = default;

    QStringList favorites() const;
    bool seeded() const;

    Q_INVOKABLE void setFavorites(const QStringList &packages);
    Q_INVOKABLE void toggle(const QString &packageName);
    Q_INVOKABLE void markSeeded();

signals:
    void favoritesChanged(const QStringList &packages);
    void seededChanged(bool seeded);

private:
    QSettings *m_settings;
    mutable QMutex m_mutex;
    static constexpr const char *FAVORITES_KEY = "favorites";
    static constexpr const char *SEEDED_KEY = "seeded";
};

} // namespace Stratara::System