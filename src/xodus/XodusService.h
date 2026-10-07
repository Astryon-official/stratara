#pragma once

#include <QObject>
#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QSettings>

namespace Stratara::Xodus {

class XodusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.astyrion.Xodus")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.astyrion.Xodus\">\n"
"    <method name=\"GetVersion\">\n"
"      <arg name=\"version\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"GetSystemInfo\">\n"
"      <arg name=\"info\" type=\"a{sv}\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"RequestPowerAction\">\n"
"      <arg name=\"action\" type=\"i\" direction=\"in\"/>\n"
"      <arg name=\"success\" type=\"b\" direction=\"out\"/>\n"
"      <arg name=\"error\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"SetHostname\">\n"
"      <arg name=\"hostname\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"success\" type=\"b\" direction=\"out\"/>\n"
"      <arg name=\"error\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"SetLocale\">\n"
"      <arg name=\"category\" type=\"i\" direction=\"in\"/>\n"
"      <arg name=\"value\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"success\" type=\"b\" direction=\"out\"/>\n"
"      <arg name=\"error\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"SetTimezone\">\n"
"      <arg name=\"timezone\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"success\" type=\"b\" direction=\"out\"/>\n"
"      <arg name=\"error\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"SetKeyboardLayout\">\n"
"      <arg name=\"layout\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"variant\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"success\" type=\"b\" direction=\"out\"/>\n"
"      <arg name=\"error\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"GetSetting\">\n"
"      <arg name=\"key\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"defaultValue\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"value\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"SetSetting\">\n"
"      <arg name=\"key\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"value\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"success\" type=\"b\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"ListSystemdUnits\">\n"
"      <arg name=\"type\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"units\" type=\"a{sv}\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"GetSystemdUnitStatus\">\n"
"      <arg name=\"unitName\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"status\" type=\"a{sv}\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"ControlSystemdUnit\">\n"
"      <arg name=\"unitName\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"action\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"success\" type=\"b\" direction=\"out\"/>\n"
"      <arg name=\"error\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"RegisterService\">\n"
"      <arg name=\"serviceName\" type=\"s\" direction=\"in\"/>\n"
"      <arg name=\"config\" type=\"a{sv}\" direction=\"in\"/>\n"
"      <arg name=\"success\" type=\"b\" direction=\"out\"/>\n"
"      <arg name=\"error\" type=\"s\" direction=\"out\"/>\n"
"    </method>\n"
"    <signal name=\"SystemInfoChanged\">\n"
"      <arg name=\"info\" type=\"a{sv}\"/>\n"
"    </signal>\n"
"    <signal name=\"SettingChanged\">\n"
"      <arg name=\"key\" type=\"s\"/>\n"
"      <arg name=\"value\" type=\"s\"/>\n"
"    </signal>\n"
"    <signal name=\"SystemdUnitStateChanged\">\n"
"      <arg name=\"unitName\" type=\"s\"/>\n"
"      <arg name=\"state\" type=\"s\"/>\n"
"    </signal>\n"
"  </interface>\n"
"")

public:
    explicit XodusAdaptor(QObject *parent);
    virtual ~XodusAdaptor() = default;

public Q_SLOTS:
    QString GetVersion();
    QVariantMap GetSystemInfo();
    QVariantMap RequestPowerAction(int action);
    QVariantMap SetHostname(const QString &hostname);
    QVariantMap SetLocale(int category, const QString &value);
    QVariantMap SetTimezone(const QString &timezone);
    QVariantMap SetKeyboardLayout(const QString &layout, const QString &variant);
    QString GetSetting(const QString &key, const QString &defaultValue);
    QVariantMap SetSetting(const QString &key, const QString &value);
    QVariantMap ListSystemdUnits(const QString &type);
    QVariantMap GetSystemdUnitStatus(const QString &unitName);
    QVariantMap ControlSystemdUnit(const QString &unitName, const QString &action);
    QVariantMap RegisterService(const QString &serviceName, const QVariantMap &config);

Q_SIGNALS:
    void SystemInfoChanged(const QVariantMap &info);
    void SettingChanged(const QString &key, const QString &value);
    void SystemdUnitStateChanged(const QString &unitName, const QString &state);
};

class XodusService : public QObject
{
    Q_OBJECT

public:
    explicit XodusService(QObject *parent = nullptr);
    ~XodusService() override = default;

    bool initialize();
    void shutdown();

private:
    XodusAdaptor *m_adaptor = nullptr;
    QSettings *m_settings = nullptr;
};

} // namespace Stratara::Xodus