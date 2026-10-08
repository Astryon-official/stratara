#include "MediaManager.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QDebug>

namespace Stratara::System {

MediaManager::MediaManager(QObject *parent)
    : QObject(parent)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(5000);
    connect(m_updateTimer, &QTimer::timeout, this, &MediaManager::refreshPlayers);
    m_updateTimer->start();

    m_positionTimer = new QTimer(this);
    m_positionTimer->setInterval(1000);
    connect(m_positionTimer, &QTimer::timeout, this, [this]() {
        if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
            QVariant posVariant = m_activePlayerInterface->property("Position");
            if (posVariant.isValid()) {
                m_position = posVariant.toLongLong();
                emit positionChanged(m_position);
            }
        }
    });
    m_positionTimer->start();

    initDBus();

    // Initial player discovery at startup
    discoverPlayers();
}

void MediaManager::initDBus()
{
    // Monitor MPRIS players on session bus
    QDBusConnection::sessionBus().connect(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "NameOwnerChanged",
        this,
        SLOT(onNameOwnerChanged(QString,QString,QString))
    );
}

void MediaManager::onNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    if (name.startsWith("org.mpris.MediaPlayer2.")) {
        if (!newOwner.isEmpty()) {
            discoverPlayers();
        } else {
            refreshPlayers();
        }
    }
}

bool MediaManager::hasActivePlayer() const
{
    return m_hasActivePlayer;
}

QString MediaManager::activePlayer() const
{
    return m_activePlayer;
}

QString MediaManager::title() const
{
    return m_title;
}

QString MediaManager::artist() const
{
    return m_artist;
}

QString MediaManager::album() const
{
    return m_album;
}

QString MediaManager::artUrl() const
{
    return m_artUrl;
}

qint64 MediaManager::position() const
{
    return m_position;
}

qint64 MediaManager::duration() const
{
    return m_duration;
}

QString MediaManager::playbackStatus() const
{
    return m_playbackStatus;
}

double MediaManager::volume() const
{
    return m_volume;
}

bool MediaManager::shuffle() const
{
    return m_shuffle;
}

int MediaManager::loopStatus() const
{
    return m_loopStatus;
}

QList<QVariantMap> MediaManager::players() const
{
    return m_players;
}

void MediaManager::refreshPlayers()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    // Timer-based refresh: check if the current active player is still running
    // and update player info if valid
    if (!m_activePlayerBus.isEmpty()) {
        QDBusInterface playerInterface(
            m_activePlayerBus,
            "/org/mpris/MediaPlayer2",
            "org.mpris.MediaPlayer2.Player",
            QDBusConnection::sessionBus(),
            this
        );

        if (playerInterface.isValid()) {
            QVariantMap playerInfo = playerToMap(m_activePlayerBus);
            if (!playerInfo.isEmpty()) {
                m_players.clear();
                m_players.append(playerInfo);
            }
        }
    }

    emit playersChanged();
}

void MediaManager::discoverPlayers()
{
    refreshPlayers();
}

void MediaManager::updateActivePlayer()
{
    if (m_activePlayerBus.isEmpty()) return;

    if (m_activePlayerInterface) {
        m_activePlayerInterface->deleteLater();
    }

    m_activePlayerInterface = new QDBusInterface(
        m_activePlayerBus,
        "/org/mpris/MediaPlayer2",
        "org.mpris.MediaPlayer2.Player",
        QDBusConnection::sessionBus(),
        this
    );

    if (m_activePlayerInterface->isValid()) {
        connect(m_activePlayerInterface, SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)),
                this, SLOT(onPlayerPropertiesChanged(QString,QVariantMap,QStringList)));

        updatePlayerInfo();
    }
}

void MediaManager::updatePlayerInfo()
{
    if (!m_activePlayerInterface || !m_activePlayerInterface->isValid()) return;

    QVariant metadataVariant = m_activePlayerInterface->property("Metadata");
    if (metadataVariant.isValid()) {
        QVariantMap metadata = metadataVariant.toMap();

        QString newTitle = metadata.value("xesam:title").toString();
        QString newArtist = metadata.value("xesam:artist").toStringList().join(", ");
        QString newAlbum = metadata.value("xesam:album").toString();
        QString newArtUrl = metadata.value("mpris:artUrl").toString();

        if (newTitle != m_title) { m_title = newTitle; emit metadataChanged(); }
        if (newArtist != m_artist) { m_artist = newArtist; emit metadataChanged(); }
        if (newAlbum != m_album) { m_album = newAlbum; emit metadataChanged(); }
        if (newArtUrl != m_artUrl) { m_artUrl = newArtUrl; emit metadataChanged(); }

        m_duration = metadata.value("mpris:length").toLongLong();
        emit durationChanged(m_duration);
    }

    QVariant statusVariant = m_activePlayerInterface->property("PlaybackStatus");
    if (statusVariant.isValid()) {
        QString newStatus = statusVariant.toString();
        if (newStatus != m_playbackStatus) {
            m_playbackStatus = newStatus;
            emit playbackStatusChanged(m_playbackStatus);
        }
    }

    QVariant volumeVariant = m_activePlayerInterface->property("Volume");
    if (volumeVariant.isValid()) {
        m_volume = volumeVariant.toDouble();
        emit volumeChanged(m_volume);
    }

    QVariant shuffleVariant = m_activePlayerInterface->property("Shuffle");
    if (shuffleVariant.isValid()) {
        m_shuffle = shuffleVariant.toBool();
        emit shuffleChanged(m_shuffle);
    }

    QVariant loopVariant = m_activePlayerInterface->property("LoopStatus");
    if (loopVariant.isValid()) {
        m_loopStatus = loopVariant.toInt();
        emit loopStatusChanged(m_loopStatus);
    }

    m_hasActivePlayer = true;
    m_activePlayer = m_activePlayerName;
    emit activePlayerChanged(m_activePlayer);
    emit hasActivePlayerChanged(true);
}

void MediaManager::onPlayerPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated)
{
    Q_UNUSED(interface);
    Q_UNUSED(invalidated);

    if (properties.contains("Metadata")) {
        updatePlayerInfo();
    }
    if (properties.contains("PlaybackStatus")) {
        m_playbackStatus = properties["PlaybackStatus"].toString();
        emit playbackStatusChanged(m_playbackStatus);
    }
    if (properties.contains("Volume")) {
        m_volume = properties["Volume"].toDouble();
        emit volumeChanged(m_volume);
    }
    if (properties.contains("Shuffle")) {
        m_shuffle = properties["Shuffle"].toBool();
        emit shuffleChanged(m_shuffle);
    }
    if (properties.contains("LoopStatus")) {
        m_loopStatus = properties["LoopStatus"].toInt();
        emit loopStatusChanged(m_loopStatus);
    }
    if (properties.contains("Position")) {
        m_position = properties["Position"].toLongLong();
        emit positionChanged(m_position);
    }
}

QVariantMap MediaManager::playerToMap(const QString &playerName)
{
    QDBusInterface playerInterface(
        playerName,
        "/org/mpris/MediaPlayer2",
        "org.mpris.MediaPlayer2.Player",
        QDBusConnection::sessionBus(),
        this
    );

    if (!playerInterface.isValid()) return {};

    QVariantMap info;
    info["name"] = playerName;

    QDBusInterface identityInterface(
        playerName,
        "/org/mpris/MediaPlayer2",
        "org.mpris.MediaPlayer2",
        QDBusConnection::sessionBus(),
        this
    );

    if (identityInterface.isValid()) {
        QVariant identityVariant = identityInterface.property("Identity");
        if (identityVariant.isValid()) {
            info["identity"] = identityVariant.toString();
        }

        QVariant desktopEntryVariant = identityInterface.property("DesktopEntry");
        if (desktopEntryVariant.isValid()) {
            info["desktopEntry"] = desktopEntryVariant.toString();
        }
    }

    QVariant statusVariant = playerInterface.property("PlaybackStatus");
    if (statusVariant.isValid()) {
        info["status"] = statusVariant.toString();
        if (statusVariant.toString() != "Stopped") {
            m_activePlayerBus = playerName;
            m_activePlayerName = playerName.split('.').last();
        }
    }

    return info;
}

void MediaManager::play()
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("Play");
    }
}

void MediaManager::pause()
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("Pause");
    }
}

void MediaManager::playPause()
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("PlayPause");
    }
}

void MediaManager::next()
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("Next");
    }
}

void MediaManager::previous()
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("Previous");
    }
}

void MediaManager::stop()
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("Stop");
    }
}

void MediaManager::seek(qint64 position)
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("SetPosition", QVariant::fromValue(QDBusObjectPath("/org/mpris/MediaPlayer2")), position);
    }
}

void MediaManager::setVolume(double volume)
{
    volume = qBound(0.0, volume, 1.0);
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("SetVolume", volume);
    }
}

void MediaManager::setShuffle(bool shuffle)
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("SetShuffle", shuffle);
    }
}

void MediaManager::setLoopStatus(int loopStatus)
{
    if (m_activePlayerInterface && m_activePlayerInterface->isValid()) {
        m_activePlayerInterface->call("SetLoopStatus", loopStatus);
    }
}

void MediaManager::setActivePlayer(const QString &playerName)
{
    if (m_activePlayerBus == playerName) return;

    m_activePlayerBus = playerName;
    m_activePlayerName = playerName.split('.').last();
    updateActivePlayer();
}

} // namespace Stratara::System