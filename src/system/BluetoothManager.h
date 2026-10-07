#pragma once

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QTimer>
#include <QDebug>

namespace Stratara::System {

class BluetoothManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool powered READ powered NOTIFY poweredChanged)
    Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged)
    Q_PROPERTY(QString adapterAddress READ adapterAddress NOTIFY adapterAddressChanged)
    Q_PROPERTY(QList<QVariantMap> devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QList<QVariantMap> pairedDevices READ pairedDevices NOTIFY pairedDevicesChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager() override = default;

    bool powered() const;
    bool discovering() const;
    QString adapterAddress() const;
    QList<QVariantMap> devices() const;
    QList<QVariantMap> pairedDevices() const;

    Q_INVOKABLE void setPowered(bool powered);
    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();
    Q_INVOKABLE void pairDevice(const QString &deviceAddress);
    Q_INVOKABLE void unpairDevice(const QString &deviceAddress);
    Q_INVOKABLE void connectDevice(const QString &deviceAddress);
    Q_INVOKABLE void disconnectDevice(const QString &deviceAddress);
    Q_INVOKABLE void setTrusted(const QString &deviceAddress, bool trusted);

signals:
    void poweredChanged(bool powered);
    void discoveringChanged(bool discovering);
    void adapterAddressChanged(const QString &address);
    void devicesChanged();
    void pairedDevicesChanged();
    void deviceFound(const QVariantMap &device);
    void deviceConnected(const QString &address);
    void deviceDisconnected(const QString &address);
    void pairingChanged(const QString &address, bool paired);

private:
    void initDBus();
    void updateDevices();
    void onPropertyChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);
    QVariantMap deviceToMap(const QString &path);

    QDBusInterface *m_adapterInterface = nullptr;
    QTimer *m_updateTimer = nullptr;

    bool m_powered = false;
    bool m_discovering = false;
    QString m_adapterAddress;
    QString m_adapterPath;
    QList<QVariantMap> m_devices;
    QList<QVariantMap> m_pairedDevices;
};

} // namespace Stratara::System