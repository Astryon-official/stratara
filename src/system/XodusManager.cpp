#include "XodusManager.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QSysInfo>
#include <QProcess>
#include <QDateTime>
#include <QStandardPaths>

namespace Stratara::System {

XodusManager::XodusManager(QObject *parent)
    : QObject(parent)
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    m_settings = new QSettings(configDir + "/xodus.conf", QSettings::IniFormat, this);

    m_uptimeTimer = new QTimer(this);
    m_uptimeTimer->setInterval(1000);
    connect(m_uptimeTimer, &QTimer::timeout, this, &XodusManager::updateUptime);
    m_uptimeTimer->start();

    m_infoRefreshTimer = new QTimer(this);
    m_infoRefreshTimer->setInterval(30000);
    connect(m_infoRefreshTimer, &QTimer::timeout, this, &XodusManager::refreshSystemInfo);
    m_infoRefreshTimer->start();

    initDBus();
    loadSystemInfo();
    loadSettings();
    updateUptime();
}

bool XodusManager::available() const
{
    return m_available;
}

QString XodusManager::version() const
{
    return m_version;
}

QString XodusManager::hostname() const
{
    return m_hostname;
}

QString XodusManager::kernelVersion() const
{
    return m_kernelVersion;
}

QString XodusManager::distroName() const
{
    return m_distroName;
}

QString XodusManager::distroVersion() const
{
    return m_distroVersion;
}

QString XodusManager::architecture() const
{
    return m_architecture;
}

bool XodusManager::systemdAvailable() const
{
    return m_systemdAvailable;
}

QString XodusManager::bootMode() const
{
    return m_bootMode;
}

qint64 XodusManager::uptime() const
{
    return m_uptime;
}

QVariantMap XodusManager::systemInfo() const
{
    return m_systemInfo;
}

void XodusManager::initDBus()
{
    m_xodusInterface = new QDBusInterface(
        XODUS_SERVICE,
        XODUS_PATH,
        XODUS_INTERFACE,
        QDBusConnection::systemBus(),
        this
    );

    if (m_xodusInterface->isValid()) {
        m_available = true;
        qInfo() << "Xodus D-Bus service connected";
    } else {
        qWarning() << "Xodus D-Bus service not available, running in standalone mode";
        m_available = false;
    }

    initSystemdIntegration();

    emit availableChanged(m_available);
}

void XodusManager::initSystemdIntegration()
{
    m_systemdManagerInterface = new QDBusInterface(
        "org.freedesktop.systemd1",
        "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager",
        QDBusConnection::systemBus(),
        this
    );

    if (m_systemdManagerInterface->isValid()) {
        m_systemdAvailable = true;

        QDBusConnection::systemBus().connect(
            "org.freedesktop.systemd1",
            "/org/freedesktop/systemd1",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            this,
            SLOT(onSystemdPropertiesChanged(QString,QVariantMap,QStringList))
        );

        qInfo() << "systemd D-Bus interface connected";
    } else {
        m_systemdAvailable = false;
        qWarning() << "systemd D-Bus interface not available";
    }

    m_hostnamedInterface = new QDBusInterface(
        "org.freedesktop.hostname1",
        "/org/freedesktop/hostname1",
        "org.freedesktop.hostname1",
        QDBusConnection::systemBus(),
        this
    );

    m_localedInterface = new QDBusInterface(
        "org.freedesktop.locale1",
        "/org/freedesktop/locale1",
        "org.freedesktop.locale1",
        QDBusConnection::systemBus(),
        this
    );

    m_timedatedInterface = new QDBusInterface(
        "org.freedesktop.timedate1",
        "/org/freedesktop/timedate1",
        "org.freedesktop.timedate1",
        QDBusConnection::systemBus(),
        this
    );

    emit systemdAvailableChanged(m_systemdAvailable);
}

void XodusManager::onSystemdPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidated)
{
    Q_UNUSED(interface);
    Q_UNUSED(invalidated);

    if (properties.contains("JobRemoved") || properties.contains("UnitNew") || properties.contains("UnitRemoved")) {
        refreshSystemInfo();
    }
}

void XodusManager::loadSystemInfo()
{
    m_kernelVersion = QSysInfo::kernelVersion();
    m_architecture = QSysInfo::currentCpuArchitecture();

    QVariantMap osRelease = parseOsRelease();
    m_distroName = osRelease.value("NAME", "Unknown").toString();
    m_distroVersion = osRelease.value("VERSION_ID", "Unknown").toString();

    m_hostname = QSysInfo::machineHostName();
    m_bootMode = readFile("/proc/cmdline").contains("systemd") ? "systemd" : "other";

    m_systemInfo["kernel"] = m_kernelVersion;
    m_systemInfo["architecture"] = m_architecture;
    m_systemInfo["distro"] = m_distroName;
    m_systemInfo["distro_version"] = m_distroVersion;
    m_systemInfo["hostname"] = m_hostname;
    m_systemInfo["boot_mode"] = m_bootMode;
    m_systemInfo["qt_version"] = qVersion();

    emit hostnameChanged(m_hostname);
    emit kernelVersionChanged(m_kernelVersion);
    emit distroNameChanged(m_distroName);
    emit distroVersionChanged(m_distroVersion);
    emit architectureChanged(m_architecture);
    emit bootModeChanged(m_bootMode);
    emit systemInfoChanged();
}

QVariantMap XodusManager::parseOsRelease() const
{
    QVariantMap result;
    QFile file("/etc/os-release");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#')) continue;

            int eqPos = line.indexOf('=');
            if (eqPos > 0) {
                QString key = line.left(eqPos).trimmed();
                QString value = line.mid(eqPos + 1).trimmed();
                if (value.startsWith('"') && value.endsWith('"')) {
                    value = value.mid(1, value.length() - 2);
                }
                result[key] = value;
            }
        }
    }
    return result;
}

void XodusManager::loadSettings()
{
    // Load persistent settings
}

void XodusManager::updateUptime()
{
    QFile uptimeFile("/proc/uptime");
    if (uptimeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&uptimeFile);
        QString line = in.readLine();
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            bool ok = false;
            double uptimeSeconds = parts[0].toDouble(&ok);
            if (ok) {
                qint64 newUptime = static_cast<qint64>(uptimeSeconds);
                if (newUptime != m_uptime) {
                    m_uptime = newUptime;
                    emit uptimeChanged(m_uptime);
                }
            }
        }
    }
}

QString XodusManager::readFile(const QString &path) const
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        return in.readAll().trimmed();
    }
    return QString();
}

void XodusManager::requestPowerAction(PowerAction action)
{
    bool success = false;
    QString error;

    if (!m_systemdManagerInterface || !m_systemdManagerInterface->isValid()) {
        error = "systemd not available";
    } else {
        QString method;
        switch (action) {
        case PowerAction::Shutdown: method = "PowerOff"; break;
        case PowerAction::Reboot: method = "Reboot"; break;
        case PowerAction::Suspend: method = "Suspend"; break;
        case PowerAction::Hibernate: method = "Hibernate"; break;
        case PowerAction::HybridSleep: method = "HybridSleep"; break;
        }

        QDBusPendingReply<> reply = m_systemdManagerInterface->call(method, true);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, action, watcher](QDBusPendingCallWatcher *w) {
            w->deleteLater();
            QDBusPendingReply<> reply = *w;
            bool success = !reply.isError();
            QString error = success ? QString() : reply.error().message();
            emit powerActionRequested(action, success, error);
        });
        return;
    }

    emit powerActionRequested(action, success, error);
}

void XodusManager::setHostname(const QString &hostname)
{
    if (!m_hostnamedInterface || !m_hostnamedInterface->isValid()) {
        qWarning() << "hostnamed not available";
        return;
    }

    QDBusPendingReply<> reply = m_hostnamedInterface->call("SetStaticHostname", hostname, true);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, hostname](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        QDBusPendingReply<> reply = *w;
        if (!reply.isError()) {
            m_hostname = hostname;
            emit hostnameChanged(m_hostname);
            m_systemInfo["hostname"] = m_hostname;
            emit systemInfoChanged();
        } else {
            qWarning() << "Failed to set hostname:" << reply.error().message();
        }
    });
}

void XodusManager::setLocale(LocaleCategory category, const QString &value)
{
    if (!m_localedInterface || !m_localedInterface->isValid()) {
        qWarning() << "localed not available";
        return;
    }

    QString property;
    switch (category) {
    case LocaleCategory::Language: property = "Locale"; break;
    case LocaleCategory::Region: property = "Locale"; break;
    case LocaleCategory::Keyboard: property = "VConsoleKeymap"; break;
    case LocaleCategory::Timezone: property = "Timezone"; break;
    }

    QDBusPendingReply<> reply = m_localedInterface->call("SetLocale", property, value, true);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, category, value](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        if (!w->isError()) {
            emit settingChanged(QString("locale_%1").arg(static_cast<int>(category)), value);
        } else {
            qWarning() << "Failed to set locale:" << w->error().message();
        }
    });
}

void XodusManager::setTimezone(const QString &timezone)
{
    if (!m_timedatedInterface || !m_timedatedInterface->isValid()) {
        qWarning() << "timedated not available";
        return;
    }

    QDBusPendingReply<> reply = m_timedatedInterface->call("SetTimezone", timezone, true);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, timezone](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        if (!w->isError()) {
            emit settingChanged("timezone", timezone);
        } else {
            qWarning() << "Failed to set timezone:" << w->error().message();
        }
    });
}

void XodusManager::setKeyboardLayout(const QString &layout, const QString &variant)
{
    if (!m_localedInterface || !m_localedInterface->isValid()) {
        qWarning() << "localed not available";
        return;
    }

    QDBusPendingReply<> reply = m_localedInterface->call("SetVConsoleKeyboard", layout, variant, true);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, layout, variant](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        if (!w->isError()) {
            emit settingChanged("keyboard_layout", layout);
            emit settingChanged("keyboard_variant", variant);
        } else {
            qWarning() << "Failed to set keyboard layout:" << w->error().message();
        }
    });
}

QString XodusManager::getSetting(const QString &key, const QString &defaultValue)
{
    return m_settings->value(key, defaultValue).toString();
}

void XodusManager::setSetting(const QString &key, const QString &value)
{
    m_settings->setValue(key, value);
    m_settings->sync();
    emit settingChanged(key, value);
}

void XodusManager::syncSettings()
{
    m_settings->sync();
}

QVariantMap XodusManager::getSystemdUnitStatus(const QString &unitName)
{
    QVariantMap result;
    if (!m_systemdManagerInterface || !m_systemdManagerInterface->isValid()) {
        result["error"] = "systemd not available";
        return result;
    }

    QDBusPendingReply<QDBusObjectPath> reply = m_systemdManagerInterface->call("GetUnit", unitName);
    reply.waitForFinished();

    if (reply.isError()) {
        result["error"] = reply.error().message();
        return result;
    }

    QDBusObjectPath unitPath = reply.value();
    QDBusInterface unitInterface(
        "org.freedesktop.systemd1",
        unitPath.path(),
        "org.freedesktop.systemd1.Unit",
        QDBusConnection::systemBus(),
        this
    );

    if (unitInterface.isValid()) {
        result["id"] = unitInterface.property("Id").toString();
        result["description"] = unitInterface.property("Description").toString();
        result["load_state"] = unitInterface.property("LoadState").toString();
        result["active_state"] = unitInterface.property("ActiveState").toString();
        result["sub_state"] = unitInterface.property("SubState").toString();
        result["object_path"] = unitPath.path();
    }

    return result;
}

void XodusManager::startSystemdUnit(const QString &unitName)
{
    if (!m_systemdManagerInterface || !m_systemdManagerInterface->isValid()) return;

    QDBusPendingReply<QDBusObjectPath> reply = m_systemdManagerInterface->call("StartUnit", unitName, "replace");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, unitName](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        if (!w->isError()) {
            emit systemdUnitStateChanged(unitName, "starting");
        }
    });
}

void XodusManager::stopSystemdUnit(const QString &unitName)
{
    if (!m_systemdManagerInterface || !m_systemdManagerInterface->isValid()) return;

    QDBusPendingReply<QDBusObjectPath> reply = m_systemdManagerInterface->call("StopUnit", unitName, "replace");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, unitName](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        if (!w->isError()) {
            emit systemdUnitStateChanged(unitName, "stopping");
        }
    });
}

void XodusManager::restartSystemdUnit(const QString &unitName)
{
    if (!m_systemdManagerInterface || !m_systemdManagerInterface->isValid()) return;

    QDBusPendingReply<QDBusObjectPath> reply = m_systemdManagerInterface->call("RestartUnit", unitName, "replace");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, unitName](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        if (!w->isError()) {
            emit systemdUnitStateChanged(unitName, "restarting");
        }
    });
}

void XodusManager::enableSystemdUnit(const QString &unitName, bool enable)
{
    if (!m_systemdManagerInterface || !m_systemdManagerInterface->isValid()) return;

    QDBusPendingReply<QDBusObjectPath, bool, bool> reply = m_systemdManagerInterface->call(
        enable ? "EnableUnitFiles" : "DisableUnitFiles",
        QStringList{unitName}, false, true
    );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, unitName, enable](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        if (!w->isError()) {
            emit systemdUnitStateChanged(unitName, enable ? "enabled" : "disabled");
        }
    });
}

QList<QVariantMap> XodusManager::listSystemdUnits(const QString &type)
{
    QList<QVariantMap> result;
    if (!m_systemdManagerInterface || !m_systemdManagerInterface->isValid()) return result;

    QDBusPendingReply<QVariantList> reply = m_systemdManagerInterface->call("ListUnitsByPatterns", QStringList(), QStringList{type + ".*"});
    reply.waitForFinished();

    if (reply.isError()) return result;

    QVariantList units = reply.value();
    for (const QVariant &unitVariant : units) {
        QVariantList unit = unitVariant.toList();
        if (unit.size() >= 5) {
            QVariantMap unitMap;
            unitMap["name"] = unit[0].toString();
            unitMap["description"] = unit[1].toString();
            unitMap["load_state"] = unit[2].toString();
            unitMap["active_state"] = unit[3].toString();
            unitMap["sub_state"] = unit[4].toString();
            if (unit.size() > 5) unitMap["object_path"] = unit[5].toString();
            result.append(unitMap);
        }
    }

    return result;
}

void XodusManager::refreshSystemInfo()
{
    loadSystemInfo();
    updateUptime();
}

void XodusManager::registerService(const QString &serviceName, const QVariantMap &config)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(config);
    qInfo() << "Service registration requested:" << serviceName;
    // Implementation would register a Stratara service with Xodus
}

} // namespace Stratara::System