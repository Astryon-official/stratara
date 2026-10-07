#pragma once

#include <QObject>
#include <QProcess>
#include <QFileInfo>
#include <QMutex>

namespace Stratara::System {

struct InstallResult {
    enum class Type {
        Started,
        NeedsPermission,
        SignatureMismatch,
        CouldNotStart,
        UnsupportedFormat
    };

    Type type = Type::CouldNotStart;
    QString errorMessage;

    InstallResult() = default;
    explicit InstallResult(Type t) : type(t) {}

    static InstallResult started() { return InstallResult{Type::Started}; }
    static InstallResult needsPermissionResult() { return InstallResult{Type::NeedsPermission}; }
    static InstallResult signatureMismatch() { return InstallResult{Type::SignatureMismatch}; }
    static InstallResult couldNotStart(const QString &error = QString()) {
        InstallResult result;
        result.type = Type::CouldNotStart;
        result.errorMessage = error;
        return result;
    }
    static InstallResult unsupportedFormat() { return InstallResult{Type::UnsupportedFormat}; }

    bool isStarted() const { return type == Type::Started; }
    bool isNeedsPermission() const { return type == Type::NeedsPermission; }
    bool isSignatureMismatch() const { return type == Type::SignatureMismatch; }
    bool isError() const { return type != Type::Started; }
};

class UpdateInstaller : public QObject
{
    Q_OBJECT

public:
    explicit UpdateInstaller(QObject *parent = nullptr);
    ~UpdateInstaller() override;

    Q_INVOKABLE InstallResult install(const QString &filePath);
    Q_INVOKABLE bool canInstallPackages() const;
    Q_INVOKABLE void requestInstallPermission();

signals:
    void installFinished(const InstallResult &result);

private:
    InstallResult installAppImage(const QString &filePath);
    InstallResult installFlatpak(const QString &filePath);
    InstallResult installDeb(const QString &filePath);
    InstallResult installRpm(const QString &filePath);
    InstallResult installGeneric(const QString &filePath);
    bool verifySignature(const QString &filePath) const;

    mutable QMutex m_mutex;
};

} // namespace Stratara::System

Q_DECLARE_METATYPE(Stratara::System::InstallResult)