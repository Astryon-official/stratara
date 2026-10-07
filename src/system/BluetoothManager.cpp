#include "BluetoothManager.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QDebug>

namespace Stratara::System {

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(5000);
    connect(m_updateTimer, &QTimer::timeout, this, &BluetoothManager::updateDevices);
    m_updateTimer->start();

    initDBus();
}

void BluetoothManager::initDBus()
{
    // Get BlueZ adapter
    QDBusInterface managerInterface(
        "org.bluez",
        "/",
        "org.freedesktop.DBus.ObjectManager",
        QDBusConnection::systemBus(),
        this
    );

    if (managerInterface.isValid()) {
        QDBusReply<QVariantMap> objectsReply = managerInterface.call("GetManagedObjects");
        if (objectsReply.isValid()) {
            QVariantMap objects = objectsReply.value();
            for (auto it = objects.begin(); it != objects.end(); ++it) {
                QVariantMap interfaces = it.value().toMap();
                if (interfaces.contains("org.bluez.Adapter1")) {
                    m_adapterPath = it.key();
                    m_adapterAddress = interfaces["org.bluez.Adapter1"].toMap().value("Address").toString();
                    break;
                }
            }
        }
    }

    if (!m_adapterPath.isEmpty()) {
        m_adapterInterface = new QDBusInterface(
            "org.bluez",
            m_adapterPath,
            "org.bluez.Adapter1",
            QDBusConnection::systemBus(),
            this
        );

        if (m_adapterInterface->isValid()) {
            QVariant poweredVariant = m_adapterInterface->property("Powered");
            if (poweredVariant.isValid()) m_powered = poweredVariant.toBool();

            QVariant discoveringVariant = m_adapterInterface->property("Discovering");
            if (discoveringVariant.isValid()) m_discovering = discoveringVariant.toBool();

            QVariant addressVariant = m_adapterInterface->property("Address");
            if (addressVariant.isValid()) m_adapterAddress = addressVariant.toString();
        }
    }
}

void BluetoothManager::onPropertyChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(interface);
    Q_UNUSED(invalidated);

    if (changed.contains("Powered")) {
        m_powered = changed["Powered"].toBool();
        emit poweredChanged(m_powered);
    }
    if (changed.contains("Discovering")) {
        m_discovering = changed["Discovering"].toBool();
        emit discoveringChanged(m_discovering);
    }
}

bool BluetoothManager::powered() const
{
    return m_powered;
}

bool BluetoothManager::discovering() const
{
    return m_discovering;
}

QString BluetoothManager::adapterAddress() const
{
    return m_adapterAddress;
}

QList<QVariantMap> BluetoothManager::devices() const
{
    return m_devices;
}

QList<QVariantMap> BluetoothManager::pairedDevices() const
{
    return m_pairedDevices;
}

void BluetoothManager::setPowered(bool powered)
{
    if (m_powered != powered) {
        m_powered = powered;
        emit poweredChanged(m_powered);
    }
}

void BluetoothManager::startDiscovery()
{
    if (m_adapterInterface && m_adapterInterface->isValid() && !m_discovering) {
        m_adapterInterface->call("StartDiscovery");
    }
}

void BluetoothManager::stopDiscovery()
{
    if (m_adapterInterface && m_adapterInterface->isValid() && m_discovering) {
        m_adapterInterface->call("StopDiscovery");
    }
}

void BluetoothManager::pairDevice(const QString &deviceAddress)
{
    if (m_adapterInterface && m_adapterInterface->isValid()) {
        QDBusInterface managerInterface(
            "org.bluez",
            "/",
            "org.freedesktop.DBus.ObjectManager",
            QDBusConnection::systemBus(),
            this
        );

        QDBusReply<QVariantMap> objectsReply = managerInterface.call("GetManagedObjects");
        if (objectsReply.isValid()) {
            QVariantMap objects = objectsReply.value();
            for (auto it = objects.begin(); it != objects.end(); ++it) {
                QVariantMap interfaces = it.value().toMap();
                if (interfaces.contains("org.bluez.Device1")) {
                    QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();
                    if (deviceProps.value("Address").toString().toLower() == deviceAddress.toLower()) {
                        QDBusInterface deviceInterface(
                            "org.bluez",
                            it.key(),
                            "org.bluez.Device1",
                            QDBusConnection::systemBus(),
                            this
                        );
                        if (deviceInterface.isValid()) {
                            deviceInterface.call("Pair");
                        }
                        break;
                    }
                }
            }
        }
    }
}

void BluetoothManager::unpairDevice(const QString &deviceAddress)
{
    if (m_adapterInterface && m_adapterInterface->isValid()) {
        QDBusInterface managerInterface(
            "org.bluez",
            "/",
            "org.freedesktop.DBus.ObjectManager",
            QDBusConnection::systemBus(),
            this
        );

        QDBusReply<QVariantMap> objectsReply = managerInterface.call("GetManagedObjects");
        if (objectsReply.isValid()) {
            QVariantMap objects = objectsReply.value();
            for (auto it = objects.begin(); it != objects.end(); ++it) {
                QVariantMap interfaces = it.value().toMap();
                if (interfaces.contains("org.bluez.Device1")) {
                    QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();
                    if (deviceProps.value("Address").toString().toLower() == deviceAddress.toLower()) {
                        m_adapterInterface->call("RemoveDevice", it.key());
                        break;
                    }
                }
            }
        }
    }
}

void BluetoothManager::connectDevice(const QString &deviceAddress)
{
    if (m_adapterInterface && m_adapterInterface->isValid()) {
        QDBusInterface managerInterface(
            "org.bluez",
            "/",
            "org.freedesktop.DBus.ObjectManager",
            QDBusConnection::systemBus(),
            this
        );

        QDBusReply<QVariantMap> objectsReply = managerInterface.call("GetManagedObjects");
        if (objectsReply.isValid()) {
            QVariantMap objects = objectsReply.value();
            for (auto it = objects.begin(); it != objects.end(); ++it) {
                QVariantMap interfaces = it.value().toMap();
                if (interfaces.contains("org.bluez.Device1")) {
                    QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();
                    if (deviceProps.value("Address").toString().toLower() == deviceAddress.toLower()) {
                        QDBusInterface deviceInterface(
                            "org.bluez",
                            it.key(),
                            "org.bluez.Device1",
                            QDBusConnection::systemBus(),
                            this
                        );
                        if (deviceInterface.isValid()) {
                            deviceInterface.call("Connect");
                        }
                        break;
                    }
                }
            }
        }
    }
}

void BluetoothManager::disconnectDevice(const QString &deviceAddress)
{
    if (m_adapterInterface && m_adapterInterface->isValid()) {
        QDBusInterface managerInterface(
            "org.bluez",
            "/",
            "org.freedesktop.DBus.ObjectManager",
            QDBusConnection::systemBus(),
            this
        );

        QDBusReply<QVariantMap> objectsReply = managerInterface.call("GetManagedObjects");
        if (objectsReply.isValid()) {
            QVariantMap objects = objectsReply.value();
            for (auto it = objects.begin(); it != objects.end(); ++it) {
                QVariantMap interfaces = it.value().toMap();
                if (interfaces.contains("org.bluez.Device1")) {
                    QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();
                    if (deviceProps.value("Address").toString().toLower() == deviceAddress.toLower()) {
                        QDBusInterface deviceInterface(
                            "org.bluez",
                            it.key(),
                            "org.bluez.Device1",
                            QDBusConnection::systemBus(),
                            this
                        );
                        if (deviceInterface.isValid()) {
                            deviceInterface.call("Disconnect");
                        }
                        break;
                    }
                }
            }
        }
    }
}

void BluetoothManager::setTrusted(const QString &deviceAddress, bool trusted)
{
    QDBusInterface managerInterface(
        "org.bluez",
        "/",
        "org.freedesktop.DBus.ObjectManager",
        QDBusConnection::systemBus(),
        this
    );

    QDBusReply<QVariantMap> objectsReply = managerInterface.call("GetManagedObjects");
    if (objectsReply.isValid()) {
        QVariantMap objects = objectsReply.value();
        for (auto it = objects.begin(); it != objects.end(); ++it) {
            QVariantMap interfaces = it.value().toMap();
            if (interfaces.contains("org.bluez.Device1")) {
                QVariantMap deviceProps = interfaces["org.bluez.Device1"].toMap();
                if (deviceProps.value("Address").toString().toLower() == deviceAddress.toLower()) {
                    QDBusInterface deviceInterface(
                        "org.bluez",
                        it.key(),
                        "org.bluez.Device1",
                        QDBusConnection::systemBus(),
                        this
                    );
                    if (deviceInterface.isValid()) {
                        deviceInterface.call("Set", "org.bluez.Device1", "Trusted", QVariant(trusted));
                    }
                    break;
                }
            }
        }
    }
}

void BluetoothManager::updateDevices()
{
    m_devices.clear();
    m_pairedDevices.clear();

    QDBusInterface managerInterface(
        "org.bluez",
        "/",
        "org.freedesktop.DBus.ObjectManager",
        QDBusConnection::systemBus(),
        this
    );

    QDBusReply<QVariantMap> objectsReply = managerInterface.call("GetManagedObjects");
    if (objectsReply.isValid()) {
        QVariantMap objects = objectsReply.value();
        for (auto it = objects.begin(); it != objects.end(); ++it) {
            QVariantMap interfaces = it.value().toMap();
            if (interfaces.contains("org.bluez.Device1")) {
                QVariantMap device = deviceToMap(it.key());
                if (!device.isEmpty()) {
                    m_devices.append(device);
                    if (device["paired"].toBool()) {
                        m_pairedDevices.append(device);
                    }
                }
            }
        }
    }

    emit devicesChanged();
    emit pairedDevicesChanged();
}

QVariantMap BluetoothManager::deviceToMap(const QString &path)
{
    QDBusInterface deviceInterface(
        "org.bluez",
        path,
        "org.bluez.Device1",
        QDBusConnection::systemBus(),
        this
    );

    if (!deviceInterface.isValid()) return {};

    QVariantMap device;
    device["path"] = path;
    device["address"] = deviceInterface.property("Address").toString();
    device["name"] = deviceInterface.property("Name").toString();
    device["alias"] = deviceInterface.property("Alias").toString();
    device["paired"] = deviceInterface.property("Paired").toBool();
    device["connected"] = deviceInterface.property("Connected").toBool();
    device["trusted"] = deviceInterface.property("Trusted").toBool();
    device["rssi"] = deviceInterface.property("RSSI").toInt();
    device["icon"] = deviceInterface.property("Icon").toString();
    device["uuids"] = deviceInterface.property("UUIDs").toStringList();

    return device;
}

} // namespace Stratara::System