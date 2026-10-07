#pragma once

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QIODevice>

namespace Stratara::System {

class PowerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool onBattery READ onBattery NOTIFY onBatteryChanged)
    Q_PROPERTY(int batteryPercentage READ batteryPercentage NOTIFY batteryPercentageChanged)
    Q_PROPERTY(int batteryTimeRemaining READ batteryTimeRemaining NOTIFY batteryTimeRemainingChanged)
    Q_PROPERTY(QString batteryState READ batteryState NOTIFY batteryStateChanged)
    Q_PROPERTY(bool lidClosed READ lidClosed NOTIFY lidClosedChanged)
    Q_PROPERTY(int brightness READ brightness NOTIFY brightnessChanged)
    Q_PROPERTY(int maxBrightness READ maxBrightness NOTIFY maxBrightnessChanged)

public:
    explicit PowerManager(QObject *parent = nullptr);
    ~PowerManager() override = default;

    bool onBattery() const;
    int batteryPercentage() const;
    int batteryTimeRemaining() const;
    QString batteryState() const;
    bool lidClosed() const;
    int brightness() const;
    int maxBrightness() const;

    Q_INVOKABLE void setBrightness(int brightness);
    Q_INVOKABLE void suspend();
    Q_INVOKABLE void hibernate();
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void reboot();

signals:
    void onBatteryChanged(bool onBattery);
    void batteryPercentageChanged(int percentage);
    void batteryTimeRemainingChanged(int seconds);
    void batteryStateChanged(const QString &state);
    void lidClosedChanged(bool closed);
    void brightnessChanged(int brightness);
    void maxBrightnessChanged(int maxBrightness);
    void lowBatteryWarning(int percentage);
    void criticalBatteryWarning(int percentage);

private:
    void initDBus();
    void updateBatteryInfo();
    void checkLidState();
    void updateBrightness();
    QString batteryStateToString(int state) const;

    QDBusInterface *m_upowerInterface = nullptr;
    QDBusInterface *m_logindInterface = nullptr;
    QTimer *m_updateTimer = nullptr;

    bool m_onBattery = false;
    int m_batteryPercentage = 100;
    int m_batteryTimeRemaining = -1;
    QString m_batteryState = "Unknown";
    bool m_lidClosed = false;
    int m_brightness = 100;
    int m_maxBrightness = 100;
    QString m_batteryPath;
};

} // namespace Stratara::System