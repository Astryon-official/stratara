#include "PowerManager.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QIODevice>

namespace Stratara::System {

PowerManager::PowerManager(QObject *parent)
    : QObject(parent)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(10000);
    connect(m_updateTimer, &QTimer::timeout, this, &PowerManager::updateBatteryInfo);
    m_updateTimer->start();

    initDBus();
    updateBatteryInfo();
    checkLidState();
    updateBrightness();
}

void PowerManager::initDBus()
{
    // UPower for battery info
    m_upowerInterface = new QDBusInterface(
        "org.freedesktop.UPower",
        "/org/freedesktop/UPower",
        "org.freedesktop.UPower",
        QDBusConnection::systemBus(),
        this
    );

    if (m_upowerInterface->isValid()) {
        // Enumerate devices to find battery
        QDBusReply<QList<QDBusObjectPath>> devicesReply = m_upowerInterface->call("EnumerateDevices");
        if (devicesReply.isValid()) {
            QList<QDBusObjectPath> devices = devicesReply.value();
            for (const QDBusObjectPath &devicePath : devices) {
                QDBusInterface deviceInterface(
                    "org.freedesktop.UPower",
                    devicePath.path(),
                    "org.freedesktop.UPower.Device",
                    QDBusConnection::systemBus(),
                    this
                );
                if (deviceInterface.isValid()) {
                    QVariant typeVariant = deviceInterface.property("Type");
                    if (typeVariant.isValid() && typeVariant.toUInt() == 2) { // Battery
                        m_batteryPath = devicePath.path();
                        break;
                    }
                }
            }
        }
    }

    // LoginD for power management
    m_logindInterface = new QDBusInterface(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus(),
        this
    );
}

bool PowerManager::onBattery() const
{
    return m_onBattery;
}

int PowerManager::batteryPercentage() const
{
    return m_batteryPercentage;
}

int PowerManager::batteryTimeRemaining() const
{
    return m_batteryTimeRemaining;
}

QString PowerManager::batteryState() const
{
    return m_batteryState;
}

bool PowerManager::lidClosed() const
{
    return m_lidClosed;
}

int PowerManager::brightness() const
{
    return m_brightness;
}

int PowerManager::maxBrightness() const
{
    return m_maxBrightness;
}

void PowerManager::updateBatteryInfo()
{
    if (m_batteryPath.isEmpty()) {
        // Try to find battery again
        if (m_upowerInterface && m_upowerInterface->isValid()) {
            QDBusReply<QList<QDBusObjectPath>> devicesReply = m_upowerInterface->call("EnumerateDevices");
            if (devicesReply.isValid()) {
                QList<QDBusObjectPath> devices = devicesReply.value();
                for (const QDBusObjectPath &devicePath : devices) {
                    QDBusInterface deviceInterface(
                        "org.freedesktop.UPower",
                        devicePath.path(),
                        "org.freedesktop.UPower.Device",
                        QDBusConnection::systemBus(),
                        this
                    );
                    if (deviceInterface.isValid()) {
                        QVariant typeVariant = deviceInterface.property("Type");
                        if (typeVariant.isValid() && typeVariant.toUInt() == 2) { // Battery
                            m_batteryPath = devicePath.path();
                        }
                    }
                }
            }
        }
    }

    if (!m_batteryPath.isEmpty()) {
        QDBusInterface batteryInterface(
            "org.freedesktop.UPower",
            m_batteryPath,
            "org.freedesktop.UPower.Device",
            QDBusConnection::systemBus(),
            this
        );

        if (batteryInterface.isValid()) {
            QVariant percentageVariant = batteryInterface.property("Percentage");
            if (percentageVariant.isValid()) {
                int newPercentage = qRound(percentageVariant.toDouble());
                if (newPercentage != m_batteryPercentage) {
                    m_batteryPercentage = newPercentage;
                    emit batteryPercentageChanged(m_batteryPercentage);

                    // Emit warnings
                    if (m_batteryPercentage <= 5) {
                        emit criticalBatteryWarning(m_batteryPercentage);
                    } else if (m_batteryPercentage <= 15) {
                        emit lowBatteryWarning(m_batteryPercentage);
                    }
                }
            }

            QVariant stateVariant = batteryInterface.property("State");
            if (stateVariant.isValid()) {
                QString newState = batteryStateToString(stateVariant.toInt());
                if (newState != m_batteryState) {
                    m_batteryState = newState;
                    emit batteryStateChanged(m_batteryState);
                }
            }

            QVariant timeVariant = batteryInterface.property("TimeToEmpty");
            if (timeVariant.isValid()) {
                int newTime = timeVariant.toInt();
                if (newTime != m_batteryTimeRemaining) {
                    m_batteryTimeRemaining = newTime;
                    emit batteryTimeRemainingChanged(m_batteryTimeRemaining);
                }
            }

            QVariant energyVariant = batteryInterface.property("Energy");
            QVariant energyFullVariant = batteryInterface.property("EnergyFull");
            QVariant energyRateVariant = batteryInterface.property("EnergyRate");
            QVariant onlineVariant = batteryInterface.property("Online");

            if (onlineVariant.isValid()) {
                m_onBattery = !onlineVariant.toBool(); // Online = on AC, so !Online = on battery
                emit onBatteryChanged(m_onBattery);
            }

            // Calculate time remaining if not provided
            if (energyVariant.isValid() && energyRateVariant.isValid() && energyRateVariant.toDouble() > 0) {
                double timeHours = energyVariant.toDouble() / energyRateVariant.toDouble();
                m_batteryTimeRemaining = qRound(timeHours * 3600);
                emit batteryTimeRemainingChanged(m_batteryTimeRemaining);
            }
        }
    }

    checkLidState();
}

void PowerManager::checkLidState()
{
    // Check lid state via logind
    if (m_logindInterface && m_logindInterface->isValid()) {
        QVariant lidVariant = m_logindInterface->property("LidClosed");
        if (lidVariant.isValid()) {
            bool newLidClosed = lidVariant.toBool();
            if (newLidClosed != m_lidClosed) {
                m_lidClosed = newLidClosed;
                emit lidClosedChanged(m_lidClosed);
            }
        }
    }
}

void PowerManager::updateBrightness()
{
    // Try to get brightness from sysfs
    QFile brightnessFile("/sys/class/backlight/amdgpu_bl0/brightness");
    if (!brightnessFile.exists()) {
        brightnessFile.setFileName("/sys/class/backlight/intel_backlight/brightness");
    }
    if (!brightnessFile.exists()) {
        // Try generic
        QDir backlightDir("/sys/class/backlight/");
        QStringList entries = backlightDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (!entries.isEmpty()) {
            brightnessFile.setFileName("/sys/class/backlight/" + entries.first() + "/brightness");
        }
    }

    if (brightnessFile.exists() && brightnessFile.open(QIODevice::ReadOnly)) {
        QString content = brightnessFile.readAll().trimmed();
        bool ok;
        int brightness = content.toInt(&ok);
        if (ok && brightness != m_brightness) {
            m_brightness = brightness;
            emit brightnessChanged(m_brightness);
        }
    }

    QFile maxBrightnessFile("/sys/class/backlight/amdgpu_bl0/max_brightness");
    if (!maxBrightnessFile.exists()) {
        maxBrightnessFile.setFileName("/sys/class/backlight/intel_backlight/max_brightness");
    }
    if (!maxBrightnessFile.exists()) {
        QDir backlightDir("/sys/class/backlight/");
        QStringList entries = backlightDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (!entries.isEmpty()) {
            maxBrightnessFile.setFileName("/sys/class/backlight/" + entries.first() + "/max_brightness");
        }
    }

    if (maxBrightnessFile.exists() && maxBrightnessFile.open(QIODevice::ReadOnly)) {
        QString content = maxBrightnessFile.readAll().trimmed();
        bool ok;
        int maxBrightness = content.toInt(&ok);
        if (ok && maxBrightness != m_maxBrightness) {
            m_maxBrightness = maxBrightness;
            emit maxBrightnessChanged(m_maxBrightness);
        }
    }
}

void PowerManager::setBrightness(int brightness)
{
    brightness = qBound(0, brightness, m_maxBrightness);

    QFile brightnessFile("/sys/class/backlight/amdgpu_bl0/brightness");
    if (!brightnessFile.exists()) {
        brightnessFile.setFileName("/sys/class/backlight/intel_backlight/brightness");
    }
    if (!brightnessFile.exists()) {
        QDir backlightDir("/sys/class/backlight/");
        QStringList entries = backlightDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (!entries.isEmpty()) {
            brightnessFile.setFileName("/sys/class/backlight/" + entries.first() + "/brightness");
        }
    }

    if (brightnessFile.open(QIODevice::WriteOnly)) {
        brightnessFile.write(QByteArray::number(brightness));
        m_brightness = brightness;
        emit brightnessChanged(m_brightness);
    }
}

void PowerManager::suspend()
{
    if (m_logindInterface && m_logindInterface->isValid()) {
        m_logindInterface->call("Suspend", true);
    }
}

void PowerManager::hibernate()
{
    if (m_logindInterface && m_logindInterface->isValid()) {
        m_logindInterface->call("Hibernate", true);
    }
}

void PowerManager::shutdown()
{
    if (m_logindInterface && m_logindInterface->isValid()) {
        m_logindInterface->call("PowerOff", true);
    }
}

void PowerManager::reboot()
{
    if (m_logindInterface && m_logindInterface->isValid()) {
        m_logindInterface->call("Reboot", true);
    }
}

QString PowerManager::batteryStateToString(int state) const
{
    // UPower states: 1=Unknown, 2=Charging, 3=Discharging, 4=Empty, 5=FullyCharged, 6=PendingCharge, 7=PendingDischarge
    switch (state) {
    case 1: return "Unknown";
    case 2: return "Charging";
    case 3: return "Discharging";
    case 4: return "Empty";
    case 5: return "Full";
    case 6: return "Pending Charge";
    case 7: return "Pending Discharge";
    default: return "Unknown";
    }
}

} // namespace Stratara::System