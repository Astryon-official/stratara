#pragma once

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QList>
#include <QVariantMap>

namespace Stratara::System {

class NotificationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(QList<QVariantMap> history READ history NOTIFY historyChanged)
    Q_PROPERTY(bool doNotDisturb READ doNotDisturb WRITE setDoNotDisturb NOTIFY doNotDisturbChanged)

public:
    enum class Urgency {
        Low = 0,
        Normal = 1,
        Critical = 2
    };
    Q_ENUM(Urgency)

    explicit NotificationManager(QObject *parent = nullptr);
    ~NotificationManager() override = default;

    bool available() const;
    QList<QVariantMap> history() const;
    bool doNotDisturb() const;

    Q_INVOKABLE void sendNotification(const QString &appName, const QString &summary,
                                      const QString &body = QString(),
                                      const QString &icon = QString(),
                                      int timeout = 5000,
                                      Urgency urgency = Urgency::Normal,
                                      const QString &category = QString(),
                                      const QVariantMap &hints = QVariantMap());
    Q_INVOKABLE void closeNotification(uint id);
    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE void setDoNotDisturb(bool enabled);

    Q_INVOKABLE int getCapabilities();

signals:
    void availableChanged(bool available);
    void historyChanged();
    void doNotDisturbChanged(bool enabled);
    void notificationReceived(const QVariantMap &notification);
    void notificationClosed(uint id, uint reason);
    void actionInvoked(uint id, const QString &actionKey);

private:
    void initDBus();
    void onNotificationClosed(uint id, uint reason);
    void onActionInvoked(uint id, const QString &actionKey);
    void addToHistory(const QVariantMap &notification);
    QVariantMap buildNotification(const QString &appName, const QString &summary,
                                  const QString &body, const QString &icon,
                                  int timeout, Urgency urgency,
                                  const QString &category, const QVariantMap &hints);

    QDBusInterface *m_notificationInterface = nullptr;
    QDBusInterface *m_notificationDaemonInterface = nullptr;

    bool m_available = false;
    QList<QVariantMap> m_history;
    bool m_doNotDisturb = false;
    uint m_nextId = 1;
    static const int MAX_HISTORY = 100;
};

} // namespace Stratara::System