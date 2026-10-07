#include "Application.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace Stratara::Core {

Application::Application(QObject *parent)
    : QObject(parent)
    , m_version("0.1.0")
    , m_organizationName("Astryon")
    , m_applicationName("Stratara")
{
    m_configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/stratara";
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/stratara";
    m_cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/stratara";
}

QString Application::version() const
{
    return m_version;
}

QString Application::organizationName() const
{
    return m_organizationName;
}

QString Application::applicationName() const
{
    return m_applicationName;
}

QString Application::configPath() const
{
    return m_configPath;
}

QString Application::dataPath() const
{
    return m_dataPath;
}

QString Application::cachePath() const
{
    return m_cachePath;
}

void Application::initialize()
{
    if (m_initialized) {
        return;
    }

    // Ensure directories exist
    QDir().mkpath(m_configPath);
    QDir().mkpath(m_dataPath);
    QDir().mkpath(m_cachePath);

    qInfo() << "Stratara initialized";
    qInfo() << "Config:" << m_configPath;
    qInfo() << "Data:" << m_dataPath;
    qInfo() << "Cache:" << m_cachePath;

    m_initialized = true;
    emit initialized();
}

void Application::shutdown()
{
    if (!m_initialized) {
        return;
    }

    qInfo() << "Stratara shutting down";
    emit shuttingDown();
    m_initialized = false;
}

} // namespace Stratara::Core