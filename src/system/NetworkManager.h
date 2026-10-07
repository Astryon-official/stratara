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

    Q_INVOKABLE void scanNetworks();
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password = QString());
    Q_INVOKABLE void disconnectNetwork();
    Q_INVOKABLE void setWifiEnabled(bool enabled);
    Q_INVOKABLE void setBluetoothEnabled(bool enabled);
    Q_INVOKABLE void forgetNetwork(const QString &ssid);

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

private:
    void updateNetworkState();
    void parseNetworkInterfaces();
    ConnectionType determineConnectionType(const QNetworkInterface &iface) const;
    void initDBus();
    void onDBusPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated);

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

    void updateAvailableNetworks();
};

} // namespace Stratara::System