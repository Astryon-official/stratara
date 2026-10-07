#pragma once

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QTimer>
#include <QVariantMap>
#include <QList>
#include <QSettings>
#include <QStandardPaths>

namespace Stratara::System {

class XodusManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
    Q_PROPERTY(QString hostname READ hostname NOTIFY hostnameChanged)
    Q_PROPERTY(QString kernelVersion READ kernelVersion NOTIFY kernelVersionChanged)
    Q_PROPERTY(QString distroName READ distroName NOTIFY distroNameChanged)
    Q_PROPERTY(QString distroVersion READ distroVersion NOTIFY distroVersionChanged)
    Q_PROPERTY(QString architecture READ architecture NOTIFY architectureChanged)
    Q_PROPERTY(bool systemdAvailable READ systemdAvailable NOTIFY systemdAvailableChanged)
    Q_PROPERTY(QString bootMode READ bootMode NOTIFY bootModeChanged)
    Q_PROPERTY(qint64 uptime READ uptime NOTIFY uptimeChanged)
    Q_PROPERTY(QVariantMap systemInfo READ systemInfo NOTIFY systemInfoChanged)

public:
    enum class PowerAction {
        Shutdown,
        Reboot,
        Suspend,
        Hibernate,
        HybridSleep
    };
    Q_ENUM(PowerAction)

    enum class LocaleCategory {
        Language,
        Region,
        Keyboard,
        Timezone
    };
    Q_ENUM(LocaleCategory)

    explicit XodusManager(QObject *parent = nullptr);
    ~XodusManager() override = default;

    bool available() const;
    QString version() const;
    QString hostname() const;
    QString kernelVersion() const;
    QString distroName() const;
    QString distroVersion() const;
    QString architecture() const;
    bool systemdAvailable() const;
    QString bootMode() const;
    qint64 uptime() const;
    QVariantMap systemInfo() const;

    Q_INVOKABLE void requestPowerAction(PowerAction action);
    Q_INVOKABLE void setHostname(const QString &hostname);
    Q_INVOKABLE void setLocale(LocaleCategory category, const QString &value);
    Q_INVOKABLE void setTimezone(const QString &timezone);
    Q_INVOKABLE void setKeyboardLayout(const QString &layout, const QString &variant = QString());
    Q_INVOKABLE QString getSetting(const QString &key, const QString &defaultValue = QString());
    Q_INVOKABLE void setSetting(const QString &key, const QString &value);
    Q_INVOKABLE void syncSettings();
    Q_INVOKABLE QVariantMap getSystemdUnitStatus(const QString &unitName);
    Q_INVOKABLE void startSystemdUnit(const QString &unitName);
    Q_INVOKABLE void stopSystemdUnit(const QString &unitName);
    Q_INVOKABLE void restartSystemdUnit(const QString &unitName);
    Q_INVOKABLE void enableSystemdUnit(const QString &unitName, bool enable);
    Q_INVOKABLE QList<QVariantMap> listSystemdUnits(const QString &type = "service");
    Q_INVOKABLE void refreshSystemInfo();
    Q_INVOKABLE void registerService(const QString &serviceName, const QVariantMap &config);

signals:
    void availableChanged(bool available);
    void versionChanged(const QString &version);
    void hostnameChanged(const QString &hostname);
    void kernelVersionChanged(const QString &version);
    void distroNameChanged(const QString &name);
    void distroVersionChanged(const QString &version);
    void architectureChanged(const QString &arch);
    void systemdAvailableChanged(bool available);
    void bootModeChanged(const QString &mode);
    void uptimeChanged(qint64 uptime);
    void systemInfoChanged();
    void powerActionRequested(PowerAction action, bool success, const QString &error);
    void settingChanged(const QString &key, const QString &value);
    void systemdUnitStateChanged(const QString &unitName, const QString &state);

private:
    void initDBus();
    void loadSystemInfo();
    void loadSettings();
    void updateUptime();
    QString readFile(const QString &path) const;
    QVariantMap parseOsRelease() const;
    void initSystemdIntegration();
    void onSystemdPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated);

    QDBusInterface *m_xodusInterface = nullptr;
    QDBusInterface *m_systemdManagerInterface = nullptr;
    QDBusInterface *m_hostnamedInterface = nullptr;
    QDBusInterface *m_localedInterface = nullptr;
    QDBusInterface *m_timedatedInterface = nullptr;
    QTimer *m_uptimeTimer = nullptr;
    QTimer *m_infoRefreshTimer = nullptr;

    bool m_available = false;
    QString m_version = "0.1.0";
    QString m_hostname;
    QString m_kernelVersion;
    QString m_distroName;
    QString m_distroVersion;
    QString m_architecture;
    bool m_systemdAvailable = false;
    QString m_bootMode;
    qint64 m_uptime = 0;
    QVariantMap m_systemInfo;
    QSettings *m_settings = nullptr;

    static constexpr const char *XODUS_SERVICE = "org.astyrion.Xodus";
    static constexpr const char *XODUS_PATH = "/org/astyrion/Xodus";
    static constexpr const char *XODUS_INTERFACE = "org.astyrion.Xodus";
};

} // namespace Stratara::System