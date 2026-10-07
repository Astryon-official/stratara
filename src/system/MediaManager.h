#pragma once

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QTimer>
#include <QDebug>

namespace Stratara::System {

class MediaManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasActivePlayer READ hasActivePlayer NOTIFY hasActivePlayerChanged)
    Q_PROPERTY(QString activePlayer READ activePlayer NOTIFY activePlayerChanged)
    Q_PROPERTY(QString title READ title NOTIFY metadataChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY metadataChanged)
    Q_PROPERTY(QString album READ album NOTIFY metadataChanged)
    Q_PROPERTY(QString artUrl READ artUrl NOTIFY metadataChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QString playbackStatus READ playbackStatus NOTIFY playbackStatusChanged)
    Q_PROPERTY(double volume READ volume NOTIFY volumeChanged)
    Q_PROPERTY(bool shuffle READ shuffle NOTIFY shuffleChanged)
    Q_PROPERTY(int loopStatus READ loopStatus NOTIFY loopStatusChanged)
    Q_PROPERTY(QList<QVariantMap> players READ players NOTIFY playersChanged)

public:
    explicit MediaManager(QObject *parent = nullptr);
    ~MediaManager() override = default;

    bool hasActivePlayer() const;
    QString activePlayer() const;
    QString title() const;
    QString artist() const;
    QString album() const;
    QString artUrl() const;
    qint64 position() const;
    qint64 duration() const;
    QString playbackStatus() const;
    double volume() const;
    bool shuffle() const;
    int loopStatus() const;
    QList<QVariantMap> players() const;

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(qint64 position);
    Q_INVOKABLE void setVolume(double volume);
    Q_INVOKABLE void setShuffle(bool shuffle);
    Q_INVOKABLE void setLoopStatus(int loopStatus);
    Q_INVOKABLE void setActivePlayer(const QString &playerName);
    Q_INVOKABLE void refreshPlayers();

signals:
    void hasActivePlayerChanged(bool hasActive);
    void activePlayerChanged(const QString &player);
    void metadataChanged();
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void playbackStatusChanged(const QString &status);
    void volumeChanged(double volume);
    void shuffleChanged(bool shuffle);
    void loopStatusChanged(int loopStatus);
    void playersChanged();
    void playerAppeared(const QVariantMap &player);
    void playerVanished(const QString &playerName);

private:
    void initDBus();
    void discoverPlayers();
    void updateActivePlayer();
    void updatePlayerInfo();
    void onNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void onPlayerPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated);
    QVariantMap playerToMap(const QString &playerName);

    QTimer *m_updateTimer = nullptr;
    QTimer *m_positionTimer = nullptr;

    bool m_hasActivePlayer = false;
    QString m_activePlayer;
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_artUrl;
    qint64 m_position = 0;
    qint64 m_duration = 0;
    QString m_playbackStatus = "Stopped";
    double m_volume = 1.0;
    bool m_shuffle = false;
    int m_loopStatus = 0;
    QList<QVariantMap> m_players;
    QString m_activePlayerBus;
    QString m_activePlayerName;
    QDBusInterface *m_activePlayerInterface = nullptr;
};

} // namespace Stratara::System