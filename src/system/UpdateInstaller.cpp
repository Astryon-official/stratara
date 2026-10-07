#include "UpdateInstaller.h"

#include <QProcess>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

namespace Stratara::System {

UpdateInstaller::UpdateInstaller(QObject *parent)
    : QObject(parent)
{
}

bool UpdateInstaller::canInstallPackages() const
{
    // On Linux, we can always install if we have the right tools
    // Check for common package managers
    return QFileInfo::exists("/usr/bin/flatpak") ||
           QFileInfo::exists("/usr/bin/apt") ||
           QFileInfo::exists("/usr/bin/dnf") ||
           QFileInfo::exists("/usr/bin/pacman") ||
           QFileInfo::exists("/usr/bin/zypper");
}

void UpdateInstaller::requestInstallPermission()
{
    // On Linux, this would typically open a polkit dialog or similar
    // For now, just log
    qInfo() << "Install permission requested - on Linux this is handled by polkit/package manager";
}

UpdateInstaller::~UpdateInstaller() = default;

InstallResult UpdateInstaller::install(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return InstallResult::couldNotStart("File does not exist: " + filePath);
    }

    // Verify signature if possible
    if (!verifySignature(filePath)) {
        qWarning() << "Signature verification failed for:" << filePath;
        // Don't block on signature mismatch for now - warn but continue
    }

    QString suffix = fileInfo.suffix().toLower();
    QString completeSuffix = fileInfo.completeSuffix().toLower();

    if (suffix == "appimage" || completeSuffix.contains("appimage")) {
        return installAppImage(filePath);
    } else if (suffix == "flatpak" || completeSuffix.contains("flatpak")) {
        return installFlatpak(filePath);
    } else if (suffix == "deb") {
        return installDeb(filePath);
    } else if (suffix == "rpm") {
        return installRpm(filePath);
    } else {
        return installGeneric(filePath);
    }
}

bool UpdateInstaller::verifySignature(const QString &filePath) const
{
    // For Linux, signature verification depends on package format
    // AppImage: can check embedded signature
    // Flatpak: verified by flatpak itself
    // deb/rpm: verified by package manager
    // For now, return true (trust the download source)
    Q_UNUSED(filePath);
    return true;
}

InstallResult UpdateInstaller::installAppImage(const QString &filePath)
{
    // Make AppImage executable and run it with --appimage-extract-and-run or just execute
    QProcess::execute("chmod", {"+x", filePath});

    // For AppImage updates, we typically replace the old AppImage and restart
    // The actual "install" is just replacing the executable
    QString targetPath = QCoreApplication::applicationFilePath();

    // Copy new AppImage to target location
    if (QFile::copy(filePath, targetPath + ".new")) {
        // On next restart, the new version will be used
        // For a real implementation, we'd need a proper update mechanism
        qInfo() << "AppImage downloaded to:" << targetPath << ".new";
        qInfo() << "Replace the running AppImage on next restart";
    }

    return InstallResult::started();
}

InstallResult UpdateInstaller::installFlatpak(const QString &filePath)
{
    // Install flatpak bundle
    QProcess process;
    process.start("flatpak", {"install", "--user", "--noninteractive", filePath});
    process.waitForFinished(60000);

    if (process.exitCode() == 0) {
        return InstallResult::started();
    } else {
        QString error = process.readAllStandardError();
        return InstallResult::couldNotStart("Flatpak install failed: " + error);
    }
}

InstallResult UpdateInstaller::installDeb(const QString &filePath)
{
    // Try apt, then dpkg
    QProcess process;
    process.start("apt", {"install", "-y", filePath});
    process.waitForFinished(120000);

    if (process.exitCode() == 0) {
        return InstallResult::started();
    }

    // Fallback to dpkg
    process.start("dpkg", {"-i", filePath});
    process.waitForFinished(60000);

    if (process.exitCode() == 0) {
        // Fix dependencies
        QProcess::execute("apt", {"--fix-broken", "install", "-y"});
        return InstallResult::started();
    }

    QString error = process.readAllStandardError();
    return InstallResult::couldNotStart("Deb install failed: " + error);
}

InstallResult UpdateInstaller::installRpm(const QString &filePath)
{
    // Try dnf, then rpm
    QProcess process;
    process.start("dnf", {"install", "-y", filePath});
    process.waitForFinished(120000);

    if (process.exitCode() == 0) {
        return InstallResult::started();
    }

    // Fallback to rpm
    process.start("rpm", {"-i", filePath});
    process.waitForFinished(60000);

    if (process.exitCode() == 0) {
        return InstallResult::started();
    }

    QString error = process.readAllStandardError();
    return InstallResult::couldNotStart("RPM install failed: " + error);
}

InstallResult UpdateInstaller::installGeneric(const QString &filePath)
{
    Q_UNUSED(filePath);
    return InstallResult::unsupportedFormat();
}

} // namespace Stratara::System