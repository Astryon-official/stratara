#include "NetworkManager.h"

#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QDebug>

namespace Stratara::System {

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(5000);
    connect(m_updateTimer, &QTimer::timeout, this, &NetworkManager::updateNetworkState);
    m_updateTimer->start();

    m_scanTimer = new QTimer(this);
    m_scanTimer->setInterval(30000);
    m_scanTimer->setSingleShot(true);

    initDBus();
    updateNetworkState();
}

void NetworkManager::initDBus()
{
    // Try to connect to NetworkManager via D-Bus
    m_nmInterface = new QDBusInterface(
        "org.freedesktop.NetworkManager",
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.NetworkManager",
        QDBusConnection::systemBus(),
        this
    );

    if (m_nmInterface->isValid()) {
        // Get WiFi device
        QDBusReply<QVariant> devicesReply = m_nmInterface->call("GetDevices");
        if (devicesReply.isValid()) {
            QVariantList devices = devicesReply.value().toList();
            for (const QVariant &deviceVariant : devices) {
                QDBusObjectPath devicePath = deviceVariant.value<QDBusObjectPath>();
                QDBusInterface deviceInterface(
                    "org.freedesktop.NetworkManager",
                    devicePath.path(),
                    "org.freedesktop.NetworkManager.Device",
                    QDBusConnection::systemBus(),
                    this
                );

                if (deviceInterface.isValid()) {
                    QVariant typeVariant = deviceInterface.property("DeviceType");
                    if (typeVariant.isValid() && typeVariant.toUInt() == 2) { // NM_DEVICE_TYPE_WIFI
                        m_wifiDeviceInterface = new QDBusInterface(
                            "org.freedesktop.NetworkManager",
                            devicePath.path(),
                            "org.freedesktop.NetworkManager.Device.Wireless",
                            QDBusConnection::systemBus(),
                            this
                        );
                        break;
                    }
                }
            }
        }

        // Connect to signals using PropertiesChanged
        QDBusConnection::systemBus().connect(
            "org.freedesktop.NetworkManager",
            "/org/freedesktop/NetworkManager",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            this,
            SLOT(onDBusPropertiesChanged(QString,QVariantMap,QStringList))
        );
    } else {
        qWarning() << "NetworkManager D-Bus interface not available, using fallback";
    }
}

void NetworkManager::onDBusPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated)
{
    Q_UNUSED(interface);
    Q_UNUSED(invalidated);

    if (properties.contains("State") || properties.contains("Connectivity")) {
        updateNetworkState();
    }
}

bool NetworkManager::connected() const
{
    return m_connected;
}

QString NetworkManager::connectionType() const
{
    switch (m_connectionType) {
    case ConnectionType::Ethernet: return "Ethernet";
    case ConnectionType::WiFi: return "WiFi";
    case ConnectionType::Cellular: return "Cellular";
    case ConnectionType::VPN: return "VPN";
    case ConnectionType::Disconnected: return "Disconnected";
    default: return "Unknown";
    }
}

QString NetworkManager::ipAddress() const
{
    return m_ipAddress;
}

int NetworkManager::signalStrength() const
{
    return m_signalStrength;
}

QString NetworkManager::activeSSID() const
{
    return m_activeSSID;
}

QList<QVariantMap> NetworkManager::availableNetworks() const
{
    return m_availableNetworks;
}

bool NetworkManager::wifiEnabled() const
{
    return m_wifiEnabled;
}

bool NetworkManager::bluetoothEnabled() const
{
    return m_bluetoothEnabled;
}

void NetworkManager::updateNetworkState()
{
    // First try D-Bus if available
    if (m_nmInterface && m_nmInterface->isValid()) {
        QVariant stateVariant = m_nmInterface->property("State");
        if (stateVariant.isValid()) {
            uint state = stateVariant.toUInt();
            // NM_STATE_CONNECTED_GLOBAL = 70
            m_connected = (state >= 70);
        }

        QDBusReply<QDBusObjectPath> primaryConnReply = m_nmInterface->call("Get", 
            "org.freedesktop.NetworkManager", "PrimaryConnection");
        if (primaryConnReply.isValid() && !primaryConnReply.value().path().isEmpty()) {
            QDBusInterface connInterface(
                "org.freedesktop.NetworkManager",
                primaryConnReply.value().path(),
                "org.freedesktop.NetworkManager.Connection.Active",
                QDBusConnection::systemBus(),
                this
            );

            if (connInterface.isValid()) {
                QVariant devicesVariant = connInterface.property("Devices");
                if (devicesVariant.isValid()) {
                    QVariantList devices = devicesVariant.toList();
                    for (const QVariant &deviceVariant : devices) {
                        QDBusObjectPath devicePath = deviceVariant.value<QDBusObjectPath>();
                        QDBusInterface devInterface(
                            "org.freedesktop.NetworkManager",
                            devicePath.path(),
                            "org.freedesktop.NetworkManager.Device",
                            QDBusConnection::systemBus(),
                            this
                        );
                        if (devInterface.isValid()) {
                            QVariant deviceTypeVariant = devInterface.property("DeviceType");
                            if (deviceTypeVariant.isValid()) {
                                uint deviceType = deviceTypeVariant.toUInt();
                                if (deviceType == 1) m_connectionType = ConnectionType::Ethernet;
                                else if (deviceType == 2) m_connectionType = ConnectionType::WiFi;
                            }

                            QVariant ip4Variant = devInterface.property("Ip4Config");
                            if (ip4Variant.isValid()) {
                                QDBusObjectPath ip4Path = ip4Variant.value<QDBusObjectPath>();
                                if (!ip4Path.path().isEmpty()) {
                                    QDBusInterface ip4Interface(
                                        "org.freedesktop.NetworkManager",
                                        ip4Path.path(),
                                        "org.freedesktop.NetworkManager.IP4Config",
                                        QDBusConnection::systemBus(),
                                        this
                                    );
                                    if (ip4Interface.isValid()) {
                                        QVariant addressesVariant = ip4Interface.property("Addresses");
                                        if (addressesVariant.isValid()) {
                                            QVariantList addresses = addressesVariant.toList();
                                            if (!addresses.isEmpty()) {
                                                QVariantMap addr = addresses.first().toMap();
                                                m_ipAddress = addr.value("address").toString();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Fallback to Qt network interfaces
    parseNetworkInterfaces();

    emit connectedChanged(m_connected);
    emit connectionTypeChanged(connectionType());
    emit ipAddressChanged(m_ipAddress);
    emit signalStrengthChanged(m_signalStrength);
    emit activeSSIDChanged(m_activeSSID);
}

void NetworkManager::parseNetworkInterfaces()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    bool wasConnected = m_connected;
    m_connected = false;
    m_ipAddress.clear();
    m_signalStrength = 0;
    m_connectionType = ConnectionType::Disconnected;

    for (const QNetworkInterface &iface : interfaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning) ||
            (iface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol &&
                !entry.ip().isLoopback()) {
                m_connected = true;
                m_ipAddress = entry.ip().toString();
                m_connectionType = determineConnectionType(iface);
                break;
            }
        }

        if (m_connected) break;
    }

    if (wasConnected != m_connected) {
        if (m_connected) {
            emit networkConnected(m_activeSSID);
        } else {
            emit networkDisconnected();
        }
    }
}

NetworkManager::ConnectionType NetworkManager::determineConnectionType(const QNetworkInterface &iface) const
{
    QString name = iface.name().toLower();
    QString humanName = iface.humanReadableName().toLower();

    if (name.startsWith("eth") || name.startsWith("enp") || name.startsWith("enx") ||
        humanName.contains("ethernet") || humanName.contains("wired")) {
        return ConnectionType::Ethernet;
    }
    if (name.startsWith("wlan") || name.startsWith("wlp") || name.startsWith("wlx") ||
        humanName.contains("wifi") || humanName.contains("wireless")) {
        return ConnectionType::WiFi;
    }
    if (name.startsWith("wwan") || name.startsWith("ppp") ||
        humanName.contains("mobile") || humanName.contains("cellular")) {
        return ConnectionType::Cellular;
    }
    if (name.startsWith("tun") || name.startsWith("tap") ||
        humanName.contains("vpn")) {
        return ConnectionType::VPN;
    }

    return ConnectionType::Unknown;
}

void NetworkManager::scanNetworks()
{
    if (m_wifiDeviceInterface && m_wifiDeviceInterface->isValid()) {
        QDBusPendingReply<> reply = m_wifiDeviceInterface->call("RequestScan");
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
            w->deleteLater();
            if (!w->isError()) {
                QTimer::singleShot(2000, this, &NetworkManager::updateAvailableNetworks);
            }
        });
    } else {
        updateAvailableNetworks();
    }
}

void NetworkManager::updateAvailableNetworks()
{
    m_availableNetworks.clear();

    if (m_wifiDeviceInterface && m_wifiDeviceInterface->isValid()) {
        QVariant apsVariant = m_wifiDeviceInterface->property("AccessPoints");
        if (apsVariant.isValid()) {
            QVariantList aps = apsVariant.toList();
            for (const QVariant &apVariant : aps) {
                QDBusObjectPath apPath = apVariant.value<QDBusObjectPath>();
                QDBusInterface apInterface(
                    "org.freedesktop.NetworkManager",
                    apPath.path(),
                    "org.freedesktop.NetworkManager.AccessPoint",
                    QDBusConnection::systemBus(),
                    this
                );

                if (apInterface.isValid()) {
                    QVariantMap network;
                    QByteArray ssidBytes = apInterface.property("Ssid").toByteArray();
                    network["ssid"] = QString::fromUtf8(ssidBytes);
                    network["bssid"] = apInterface.property("HwAddress").toString();
                    network["signal"] = apInterface.property("Strength").toInt();
                    network["frequency"] = apInterface.property("Frequency").toInt();
                    int wpaFlags = apInterface.property("WpaFlags").toInt();
                    int rsnFlags = apInterface.property("RsnFlags").toInt();
                    network["security"] = wpaFlags | rsnFlags;
                    network["connected"] = (network["ssid"].toString() == m_activeSSID);
                    m_availableNetworks.append(network);
                }
            }
        }
    }

    emit availableNetworksChanged();
}

void NetworkManager::connectToNetwork(const QString &ssid, const QString &password)
{
    qInfo() << "Connect to network requested:" << ssid;
    // Full implementation would use AddAndActivateConnection on NetworkManager
}

void NetworkManager::disconnectNetwork()
{
    // Would need active connection path
    qInfo() << "Disconnect network requested";
}

void NetworkManager::setWifiEnabled(bool enabled)
{
    if (m_wifiEnabled != enabled) {
        m_wifiEnabled = enabled;
        if (m_nmInterface && m_nmInterface->isValid()) {
            m_nmInterface->call("Set", "org.freedesktop.NetworkManager", "WirelessEnabled", QVariant(enabled));
        }
        emit wifiEnabledChanged(enabled);
    }
}

void NetworkManager::setBluetoothEnabled(bool enabled)
{
    if (m_bluetoothEnabled != enabled) {
        m_bluetoothEnabled = enabled;
        emit bluetoothEnabledChanged(enabled);
    }
}

void NetworkManager::forgetNetwork(const QString &ssid)
{
    qInfo() << "Forget network requested:" << ssid;
}

int NetworkManager::calculateSignalStrength(int quality, int maxQuality) const
{
    if (maxQuality <= 0) return 0;
    return qBound(0, (quality * 100) / maxQuality, 100);
}

} // namespace Stratara::System