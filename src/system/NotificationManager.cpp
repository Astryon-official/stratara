#include "NotificationManager.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusMessage>
#include <QDebug>
#include <QDateTime>

namespace Stratara::System {

NotificationManager::NotificationManager(QObject *parent)
    : QObject(parent)
{
    initDBus();
}

void NotificationManager::initDBus()
{
    // Connect to the freedesktop notification daemon
    m_notificationInterface = new QDBusInterface(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        QDBusConnection::sessionBus(),
        this
    );

    if (m_notificationInterface->isValid()) {
        // Connect to signals
        QDBusConnection::sessionBus().connect(
            "org.freedesktop.Notifications",
            "/org/freedesktop/Notifications",
            "org.freedesktop.Notifications",
            "NotificationClosed",
            this,
            SLOT(onNotificationClosed(uint,uint))
        );

        QDBusConnection::sessionBus().connect(
            "org.freedesktop.Notifications",
            "/org/freedesktop/Notifications",
            "org.freedesktop.Notifications",
            "ActionInvoked",
            this,
            SLOT(onActionInvoked(uint,QString))
        );

        m_available = true;
        qInfo() << "Notification daemon connected";
    } else {
        qWarning() << "Notification daemon not available";
        m_available = false;
    }

    emit availableChanged(m_available);
}

bool NotificationManager::available() const
{
    return m_available;
}

QList<QVariantMap> NotificationManager::history() const
{
    return m_history;
}

bool NotificationManager::doNotDisturb() const
{
    return m_doNotDisturb;
}

void NotificationManager::sendNotification(const QString &appName, const QString &summary,
                                           const QString &body, const QString &icon,
                                           int timeout, Urgency urgency,
                                           const QString &category, const QVariantMap &hints)
{
    if (!m_available || m_doNotDisturb) {
        // Still add to history even if DND is on
        QVariantMap notification = buildNotification(appName, summary, body, icon, timeout, urgency, category, hints);
        addToHistory(notification);
        emit notificationReceived(notification);
        return;
    }

    QVariantMap notification = buildNotification(appName, summary, body, icon, timeout, urgency, category, hints);

    QList<QVariant> args;
    args << appName;
    args << static_cast<uint>(m_nextId);
    args << icon;
    args << summary;
    args << body;
    args << QVariantList(); // actions
    args << hints;
    args << timeout;

    QDBusPendingReply<uint> reply = m_notificationInterface->call("Notify",
        appName,
        static_cast<uint>(m_nextId),
        icon,
        summary,
        body,
        QVariantList(), // actions
        hints,
        timeout
    );

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, notification](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        QDBusPendingReply<uint> reply = *w;
        if (!reply.isError()) {
            uint id = reply.value();
            notification["id"] = static_cast<int>(id);
            addToHistory(notification);
            emit notificationReceived(notification);
        } else {
            qWarning() << "Failed to send notification:" << reply.error().message();
        }
    });

    m_nextId++;
}

void NotificationManager::closeNotification(uint id)
{
    if (m_available && m_notificationInterface && m_notificationInterface->isValid()) {
        m_notificationInterface->call("CloseNotification", id);
    }
}

void NotificationManager::clearHistory()
{
    m_history.clear();
    emit historyChanged();
}

void NotificationManager::setDoNotDisturb(bool enabled)
{
    if (m_doNotDisturb != enabled) {
        m_doNotDisturb = enabled;
        emit doNotDisturbChanged(enabled);
    }
}

int NotificationManager::getCapabilities()
{
    if (!m_available || !m_notificationInterface || !m_notificationInterface->isValid()) {
        return 0;
    }

    QDBusPendingReply<QStringList> reply = m_notificationInterface->call("GetCapabilities");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        QDBusPendingReply<QStringList> reply = *w;
        if (!reply.isError()) {
            QStringList caps = reply.value();
            qInfo() << "Notification capabilities:" << caps;
        }
    });

    return 0; // Async, actual value comes via signal if needed
}

void NotificationManager::onNotificationClosed(uint id, uint reason)
{
    // reason: 1=expired, 2=dismissed, 3=close, 4=undefined
    emit notificationClosed(id, reason);
}

void NotificationManager::onActionInvoked(uint id, const QString &actionKey)
{
    emit actionInvoked(id, actionKey);
}

void NotificationManager::addToHistory(const QVariantMap &notification)
{
    QVariantMap entry = notification;
    entry["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    entry["id"] = notification.value("id").toInt();

    m_history.prepend(entry);
    if (m_history.size() > MAX_HISTORY) {
        m_history.removeLast();
    }
    emit historyChanged();
}

QVariantMap NotificationManager::buildNotification(const QString &appName, const QString &summary,
                                                   const QString &body, const QString &icon,
                                                   int timeout, Urgency urgency,
                                                   const QString &category, const QVariantMap &hints)
{
    QVariantMap notification;
    notification["appName"] = appName;
    notification["summary"] = summary;
    notification["body"] = body;
    notification["icon"] = icon;
    notification["timeout"] = timeout;
    notification["urgency"] = static_cast<int>(urgency);
    notification["category"] = category;
    notification["hints"] = hints;
    return notification;
}

} // namespace Stratara::System