#pragma once

#include <QObject>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QTimer>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDebug>

namespace Stratara::System {

class NetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString connectionType READ connectionType NOTIFY connectionTypeChanged)
    Q_PROPERTY(QString ipAddress READ ipAddress NOTIFY ipAddressChanged)
    Q_PROPERTY(int signalStrength READ signalStrength NOTIFY signalStrengthChanged)
    Q_PROPERTY(QString activeSSID READ activeSSID NOTIFY activeSSIDChanged)
    Q_PROPERTY(QList<QVariantMap> availableNetworks READ availableNetworks NOTIFY availableNetworksChanged)
    Q_PROPERTY(bool wifiEnabled READ wifiEnabled NOTIFY wifiEnabledChanged)
    Q_PROPERTY(bool bluetoothEnabled READ bluetoothEnabled NOTIFY bluetoothEnabledChanged)
    // Hotspot
    Q_PROPERTY(bool hotspotActive READ hotspotActive NOTIFY hotspotActiveChanged)
    Q_PROPERTY(QString hotspotSSID READ hotspotSSID NOTIFY hotspotSSIDChanged)
    Q_PROPERTY(QString hotspotPassword READ hotspotPassword NOTIFY hotspotPasswordChanged)
    Q_PROPERTY(int hotspotConnectedDevices READ hotspotConnectedDevices NOTIFY hotspotConnectedDevicesChanged)
    // VPN
    Q_PROPERTY(QList<QVariantMap> vpnConnections READ vpnConnections NOTIFY vpnConnectionsChanged)
    Q_PROPERTY(QString activeVPN READ activeVPN NOTIFY activeVPNChanged)

public:
    enum class ConnectionType {
        Unknown,
        Ethernet,
        WiFi,
        Cellular,
        VPN,
        Disconnected
    };
    Q_ENUM(ConnectionType)

    enum class VPNAuthType {
        Unknown,
        Password,
        Certificate,
        OpenVPN,
        WireGuard
    };
    Q_ENUM(VPNAuthType)

    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager() override = default;

    bool connected() const;
    QString connectionType() const;
    QString ipAddress() const;
    int signalStrength() const;
    int calculateSignalStrength(int quality, int maxQuality) const;
    QString activeSSID() const;
    QList<QVariantMap> availableNetworks() const;
    bool wifiEnabled() const;
    bool bluetoothEnabled() const;

    // Hotspot
    bool hotspotActive() const;
    QString hotspotSSID() const;
    QString hotspotPassword() const;
    int hotspotConnectedDevices() const;

    // VPN
    QList<QVariantMap> vpnConnections() const;
    QString activeVPN() const;

    Q_INVOKABLE void scanNetworks();
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password = QString());
    Q_INVOKABLE void disconnectNetwork();
    Q_INVOKABLE void setWifiEnabled(bool enabled);
    Q_INVOKABLE void setBluetoothEnabled(bool enabled);
    Q_INVOKABLE void forgetNetwork(const QString &ssid);

    // Hotspot
    Q_INVOKABLE void enableHotspot(const QString &ssid, const QString &password = QString());
    Q_INVOKABLE void disableHotspot();
    Q_INVOKABLE void setHotspotConfig(const QString &ssid, const QString &password);

    // VPN
    Q_INVOKABLE void refreshVPNConnections();
    Q_INVOKABLE void connectVPN(const QString &vpnId);
    Q_INVOKABLE void disconnectVPN();
    Q_INVOKABLE void addVPNConnection(const QString &name, VPNAuthType type, const QVariantMap &config);
    Q_INVOKABLE void removeVPNConnection(const QString &vpnId);

signals:
    void connectedChanged(bool connected);
    void connectionTypeChanged(const QString &type);
    void ipAddressChanged(const QString &ip);
    void signalStrengthChanged(int strength);
    void activeSSIDChanged(const QString &ssid);
    void availableNetworksChanged();
    void wifiEnabledChanged(bool enabled);
    void bluetoothEnabledChanged(bool enabled);
    void networkConnected(const QString &ssid);
    void networkDisconnected();

    // Hotspot
    void hotspotActiveChanged(bool active);
    void hotspotSSIDChanged(const QString &ssid);
    void hotspotPasswordChanged(const QString &password);
    void hotspotConnectedDevicesChanged(int count);
    void hotspotDeviceConnected(const QString &deviceMac);
    void hotspotDeviceDisconnected(const QString &deviceMac);

    // VPN
    void vpnConnectionsChanged();
    void activeVPNChanged(const QString &vpnId);
    void vpnConnectionStateChanged(const QString &vpnId, const QString &state);

private:
    void updateNetworkState();
    void parseNetworkInterfaces();
    ConnectionType determineConnectionType(const QNetworkInterface &iface) const;
    void initDBus();
    void onDBusPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated);
    void updateVPNConnections();

    QDBusInterface *m_nmInterface = nullptr;
    QDBusInterface *m_wifiDeviceInterface = nullptr;
    QTimer *m_updateTimer = nullptr;
    QTimer *m_scanTimer = nullptr;

    bool m_connected = false;
    ConnectionType m_connectionType = ConnectionType::Unknown;
    QString m_ipAddress;
    int m_signalStrength = 0;
    QString m_activeSSID;
    QList<QVariantMap> m_availableNetworks;
    bool m_wifiEnabled = true;
    bool m_bluetoothEnabled = true;

    // Hotspot
    bool m_hotspotActive = false;
    QString m_hotspotSSID = "Stratara";
    QString m_hotspotPassword = "";
    int m_hotspotConnectedDevices = 0;
    QDBusInterface *m_hotspotConnectionInterface = nullptr;

    // VPN
    QList<QVariantMap> m_vpnConnections;
    QString m_activeVPN;

    void updateAvailableNetworks();
    void updateHotspotState();
};

} // namespace Stratara::System