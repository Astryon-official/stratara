#include "XodusService.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QSysInfo>
#include <QProcess>
#include <QDebug>
#include <QDir>

namespace Stratara::Xodus {

XodusAdaptor::XodusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

QString XodusAdaptor::GetVersion()
{
    return "0.1.0";
}

QVariantMap XodusAdaptor::GetSystemInfo()
{
    QVariantMap info;
    info["kernel"] = QSysInfo::kernelVersion();
    info["architecture"] = QSysInfo::currentCpuArchitecture();
    info["qt_version"] = qVersion();

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
                info[key.toLower()] = value;
            }
        }
    }

    info["hostname"] = QSysInfo::machineHostName();

    QFile uptimeFile("/proc/uptime");
    if (uptimeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&uptimeFile);
        QString line = in.readLine();
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            bool ok = false;
            double uptime = parts[0].toDouble(&ok);
            if (ok) info["uptime"] = static_cast<qint64>(uptime);
        }
    }

    return info;
}

QVariantMap XodusAdaptor::RequestPowerAction(int action)
{
    QDBusInterface systemd(
        "org.freedesktop.systemd1",
        "/org/freedesktop.systemd1",
        "org.freedesktop.systemd1.Manager",
        QDBusConnection::systemBus()
    );

    QVariantMap result;

    if (!systemd.isValid()) {
        result["success"] = false;
        result["error"] = "systemd not available";
        return result;
    }

    QString method;
    switch (action) {
    case 0: method = "PowerOff"; break;
    case 1: method = "Reboot"; break;
    case 2: method = "Suspend"; break;
    case 3: method = "Hibernate"; break;
    case 4: method = "HybridSleep"; break;
    default:
        result["success"] = false;
        result["error"] = "Invalid action";
        return result;
    }

    QDBusPendingReply<> reply = systemd.call(method, true);
    reply.waitForFinished();

    if (reply.isError()) {
        result["success"] = false;
        result["error"] = reply.error().message();
    } else {
        result["success"] = true;
        result["error"] = "";
    }

    return result;
}

QVariantMap XodusAdaptor::SetHostname(const QString &hostname)
{
    QDBusInterface hostname1(
        "org.freedesktop.hostname1",
        "/org/freedesktop/hostname1",
        "org.freedesktop.hostname1",
        QDBusConnection::systemBus()
    );

    QVariantMap result;

    if (!hostname1.isValid()) {
        result["success"] = false;
        result["error"] = "hostnamed not available";
        return result;
    }

    QDBusPendingReply<> reply = hostname1.call("SetStaticHostname", hostname, true);
    reply.waitForFinished();

    if (reply.isError()) {
        result["success"] = false;
        result["error"] = reply.error().message();
    } else {
        result["success"] = true;
        result["error"] = "";
    }

    return result;
}

QVariantMap XodusAdaptor::SetLocale(int category, const QString &value)
{
    QDBusInterface locale1(
        "org.freedesktop.locale1",
        "/org/freedesktop/locale1",
        "org.freedesktop.locale1",
        QDBusConnection::systemBus()
    );

    QVariantMap result;

    if (!locale1.isValid()) {
        result["success"] = false;
        result["error"] = "localed not available";
        return result;
    }

    QString property;
    switch (category) {
    case 0: property = "Locale"; break;
    case 1: property = "Locale"; break;
    case 2: property = "VConsoleKeymap"; break;
    case 3: property = "Timezone"; break;
    default:
        result["success"] = false;
        result["error"] = "Invalid category";
        return result;
    }

    QDBusPendingReply<> reply = locale1.call("SetLocale", property, value, true);
    reply.waitForFinished();

    if (reply.isError()) {
        result["success"] = false;
        result["error"] = reply.error().message();
    } else {
        result["success"] = true;
        result["error"] = "";
    }

    return result;
}

QVariantMap XodusAdaptor::SetTimezone(const QString &timezone)
{
    QDBusInterface timedate1(
        "org.freedesktop.timedate1",
        "/org/freedesktop/timedate1",
        "org.freedesktop.timedate1",
        QDBusConnection::systemBus()
    );

    QVariantMap result;

    if (!timedate1.isValid()) {
        result["success"] = false;
        result["error"] = "timedated not available";
        return result;
    }

    QDBusPendingReply<> reply = timedate1.call("SetTimezone", timezone, true);
    reply.waitForFinished();

    if (reply.isError()) {
        result["success"] = false;
        result["error"] = reply.error().message();
    } else {
        result["success"] = true;
        result["error"] = "";
    }

    return result;
}

QVariantMap XodusAdaptor::SetKeyboardLayout(const QString &layout, const QString &variant)
{
    QDBusInterface locale1(
        "org.freedesktop.locale1",
        "/org/freedesktop/locale1",
        "org.freedesktop.locale1",
        QDBusConnection::systemBus()
    );

    QVariantMap result;

    if (!locale1.isValid()) {
        result["success"] = false;
        result["error"] = "localed not available";
        return result;
    }

    QDBusPendingReply<> reply = locale1.call("SetVConsoleKeyboard", layout, variant, true);
    reply.waitForFinished();

    if (reply.isError()) {
        result["success"] = false;
        result["error"] = reply.error().message();
    } else {
        result["success"] = true;
        result["error"] = "";
    }

    return result;
}

QString XodusAdaptor::GetSetting(const QString &key, const QString &defaultValue)
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/xodus";
    QDir().mkpath(configDir);
    QSettings settings(configDir + "/settings.conf", QSettings::IniFormat);
    return settings.value(key, defaultValue).toString();
}

QVariantMap XodusAdaptor::SetSetting(const QString &key, const QString &value)
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/xodus";
    QDir().mkpath(configDir);
    QSettings settings(configDir + "/settings.conf", QSettings::IniFormat);
    settings.setValue(key, value);
    settings.sync();
    Q_EMIT SettingChanged(key, value);

    QVariantMap result;
    result["success"] = true;
    return result;
}

QVariantMap XodusAdaptor::ListSystemdUnits(const QString &type)
{
    QDBusInterface systemd(
        "org.freedesktop.systemd1",
        "/org/freedesktop.systemd1",
        "org.freedesktop.systemd1.Manager",
        QDBusConnection::systemBus()
    );

    QVariantMap result;

    if (!systemd.isValid()) {
        return result;
    }

    QDBusPendingReply<QVariantList> reply = systemd.call("ListUnitsByPatterns", QStringList(), QStringList{type + ".*"});
    reply.waitForFinished();

    if (reply.isError()) {
        return result;
    }

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
            result[unit[0].toString()] = unitMap;
        }
    }

    return result;
}

QVariantMap XodusAdaptor::GetSystemdUnitStatus(const QString &unitName)
{
    QDBusInterface systemd(
        "org.freedesktop.systemd1",
        "/org/freedesktop.systemd1",
        "org.freedesktop.systemd1.Manager",
        QDBusConnection::systemBus()
    );

    QVariantMap result;

    if (!systemd.isValid()) {
        result["error"] = "systemd not available";
        return result;
    }

    QDBusPendingReply<QDBusObjectPath> reply = systemd.call("GetUnit", unitName);
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
        QDBusConnection::systemBus()
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

QVariantMap XodusAdaptor::ControlSystemdUnit(const QString &unitName, const QString &action)
{
    QDBusInterface systemd(
        "org.freedesktop.systemd1",
        "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager",
        QDBusConnection::systemBus()
    );

    QVariantMap result;

    if (!systemd.isValid()) {
        result["success"] = false;
        result["error"] = "systemd not available";
        return result;
    }

    QString method;
    if (action == "start") method = "StartUnit";
    else if (action == "stop") method = "StopUnit";
    else if (action == "restart") method = "RestartUnit";
    else if (action == "enable") method = "EnableUnitFiles";
    else if (action == "disable") method = "DisableUnitFiles";
    else {
        result["success"] = false;
        result["error"] = "Invalid action";
        return result;
    }

    QDBusPendingReply<> reply;
    if (action == "enable" || action == "disable") {
        reply = systemd.call(method, QStringList{unitName}, false, true);
    } else {
        reply = systemd.call(method, unitName, "replace");
    }
    reply.waitForFinished();

    if (reply.isError()) {
        result["success"] = false;
        result["error"] = reply.error().message();
    } else {
        result["success"] = true;
        result["error"] = "";
    }

    return result;
}

QVariantMap XodusAdaptor::RegisterService(const QString &serviceName, const QVariantMap &config)
{
    Q_UNUSED(config);
    qInfo() << "Service registration requested:" << serviceName;

    QVariantMap result;

    QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/xodus/services";
    QDir().mkpath(configDir);
    QSettings settings(configDir + "/" + serviceName + ".conf", QSettings::IniFormat);
    for (auto it = config.begin(); it != config.end(); ++it) {
        settings.setValue(it.key(), it.value());
    }
    settings.sync();

    result["success"] = true;
    result["error"] = "";
    return result;
}

XodusService::XodusService(QObject *parent)
    : QObject(parent)
{
}

bool XodusService::initialize()
{
    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.isConnected()) {
        qCritical() << "Cannot connect to system D-Bus";
        return false;
    }

    if (!connection.registerService("org.astyrion.Xodus")) {
        qCritical() << "Cannot register service:" << connection.lastError().message();
        return false;
    }

    m_settings = new QSettings(
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/xodus/settings.conf",
        QSettings::IniFormat,
        this
    );

    QObject *rootObject = new QObject(this);
    m_adaptor = new XodusAdaptor(rootObject);

    if (!connection.registerObject("/org/astyrion/Xodus", rootObject)) {
        qCritical() << "Cannot register object:" << connection.lastError().message();
        return false;
    }

    qInfo() << "Xodus service started successfully";
    return true;
}

void XodusService::shutdown()
{
    QDBusConnection connection = QDBusConnection::systemBus();
    if (connection.isConnected()) {
        connection.unregisterObject("/org/astyrion/Xodus");
        connection.unregisterService("org.astyrion.Xodus");
    }
}

} // namespace Stratara::Xodus