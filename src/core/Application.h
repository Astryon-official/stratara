#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

namespace Stratara::Core {

class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString organizationName READ organizationName CONSTANT)
    Q_PROPERTY(QString applicationName READ applicationName CONSTANT)
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(QString dataPath READ dataPath CONSTANT)
    Q_PROPERTY(QString cachePath READ cachePath CONSTANT)

public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override = default;

    QString version() const;
    QString organizationName() const;
    QString applicationName() const;
    QString configPath() const;
    QString dataPath() const;
    QString cachePath() const;

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void shutdown();

signals:
    void initialized();
    void shuttingDown();

private:
    QString m_version;
    QString m_organizationName;
    QString m_applicationName;
    QString m_configPath;
    QString m_dataPath;
    QString m_cachePath;
    bool m_initialized = false;
};

} // namespace Stratara::Core