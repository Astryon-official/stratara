/*
 * Stratara Shell Calamares Module
 * Custom module for Stratara post-install configuration
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <CalamaresUtils/Installer>
#include <CalamaresUtils/Logger>
#include <CalamaresUtils/System>
#include <CalamaresUtils/Process>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QStandardPaths>
#include <QSettings>
#include <QStringList>
#include <QTextStream>
#include <QDateTime>

class StrataraShellModule : public Calamares::Module
{
    Q_OBJECT

public:
    explicit StrataraShellModule( QObject* parent = nullptr );
    ~StrataraShellModule() override = default;

    QString name() const override;
    QString description() const override;
    QString summary() const override;
    bool check() const override;
    void install( const Calamares::ModuleSystem::InstallInfo& installInfo ) override;

private:
    void enableServices( const QString& rootMountPoint );
    void setupWaylandSession( const QString& rootMountPoint );
    void setupKWinConfig( const QString& rootMountPoint );
    void setupGRUBTheme( const QString& rootMountPoint );
    void setupSecureBoot( const QString& rootMountPoint );
    void enrollXodus( const QString& rootMountPoint );
    void setupDefaultUser( const QString& rootMountPoint );
    void runPostInstallScripts( const QString& rootMountPoint );
    void copyFile( const QString& source, const QString& dest );
    void copyDirectory( const QString& source, const QString& dest );
    QString readConfigValue( const QString& key, const QString& defaultValue = QString() );
    void writeConfigValue( const QString& key, const QString& value );
    bool runCommand( const QString& command, const QStringList& arguments, const QString& workingDir = QString() );
    void logInfo( const QString& message );
    void logWarning( const QString& message );
    void logError( const QString& message );
};

StrataraShellModule::StrataraShellModule( QObject* parent )
    : Calamares::Module( parent )
{
}

QString StrataraShellModule::name() const
{
    return "stratara-shell";
}

QString StrataraShellModule::description() const
{
    return "Stratara shell post-install configuration (services, Wayland session, GRUB theme)";
}

QString StrataraShellModule::summary() const
{
    return "Configuring Stratara shell services, Wayland session, and GRUB theme";
}

bool StrataraShellModule::check() const
{
    // Always run this module
    return true;
}

void StrataraShellModule::install( const Calamares::ModuleSystem::InstallInfo& installInfo )
{
    const QString rootMountPoint = installInfo.rootPath();
    logInfo( "Starting Stratara shell post-install configuration..." );

    // Enable services
    enableServices( rootMountPoint );

    // Setup Wayland session
    setupWaylandSession( rootMountPoint );

    // Setup KWin configuration
    setupKWinConfig( rootMountPoint );

    // Setup GRUB theme
    setupGRUBTheme( rootMountPoint );

    // Setup Secure Boot
    setupSecureBoot( rootMountPoint );

    // Enroll with Xodus
    enrollXodus( rootMountPoint );

    // Setup default user
    setupDefaultUser( rootMountPoint );

    // Run post-install scripts
    runPostInstallScripts( rootMountPoint );

    logInfo( "Stratara shell post-install configuration completed!" );
}

void StrataraShellModule::enableServices( const QString& rootMountPoint )
{
    logInfo( "Enabling systemd services..." );

    // System services
    QStringList systemServices = {
        "xodus.service",
        "NetworkManager.service",
        "bluetooth.service",
        "pipewire.service",
        "pipewire-pulse.service",
        "dbus.service"
    };

    for ( const QString& service : systemServices )
    {
        QStringList args = { "enable", service, "--root", rootMountPoint };
        if ( runCommand( "systemctl", args ) )
        {
            logInfo( QString( "Enabled system service: %1" ).arg( service ) );
        }
        else
        {
            logWarning( QString( "Failed to enable system service: %1" ).arg( service ) );
        }
    }

    // User services (will be enabled per-user on first login)
    QStringList userServices = {
        "stratara.service"
    };

    for ( const QString& service : userServices )
    {
        logInfo( QString( "User service %1 will be enabled on first login" ).arg( service ) );
    }
}

void StrataraShellModule::setupWaylandSession( const QString& rootMountPoint )
{
    logInfo( "Setting up Wayland session..." );

    // Create Wayland session directory
    QString sessionDir = rootMountPoint + "/usr/share/wayland-sessions";
    QDir().mkpath( sessionDir );

    // Create Stratara session desktop file
    QString sessionFile = sessionDir + "/stratara.desktop";
    QFile file( sessionFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream out( &file );
        out << "[Desktop Entry]\n";
        out << "Name=Stratara Shell\n";
        out << "Comment=Stratara Living-Room Shell (Wayland)\n";
        out << "Exec=stratara\n";
        out << "Type=Application\n";
        out << "DesktopNames=KDE\n";
        out << "X-KDE-PluginInfo-Name=stratara\n";
        out << "X-KDE-PluginInfo-Version=0.1.0\n";
        out << "X-KDE-PluginInfo-License=GPL-3.0-or-later\n";
        file.close();
        logInfo( "Created Wayland session: " + sessionFile );
    }
    else
    {
        logError( "Failed to create Wayland session file" );
    }

    #if 0
    // Create KWin Wayland session
    QString kwinSessionFile = sessionDir + "/kwin-stratara.desktop";
    QFile kwinFile( kwinSessionFile );
    if ( kwinFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream out( &kwinFile );
        out << "[Desktop Entry]\n";
        out << "Name=KWin (Stratara)\n";
        out << "Comment=KWin Wayland compositor with Stratara shell\n";
        out << "Exec=kwin_wayland --xwayland --inputmethod=qtvirtualkeyboard\n";
        out << "Type=Application\n";
        out << "DesktopNames=KDE\n";
        kwinFile.close();
        logInfo( "Created KWin session: " + kwinSessionFile );
    }
    #endif
}

void StrataraShellModule::setupKWinConfig( const QString& rootMountPoint )
{
    logInfo( "Setting up KWin configuration..." );

    // System KWin config
    QString systemConfigDir = rootMountPoint + "/etc";
    QDir().mkpath( systemConfigDir );

    QStringList kwinConfigFiles = { "kwinrc", "windowrulesrc" };
    for ( const QString& configFile : kwinConfigFiles )
    {
        QString source = "/usr/share/stratara/" + configFile;
        QString dest = systemConfigDir + "/" + configFile;
        copyFile( source, dest );
    }

    // KWin scripts
    QString scriptSource = "/usr/share/kwin/scripts/stratara-shell";
    QString scriptDest = rootMountPoint + "/usr/share/kwin/scripts/stratara-shell";
    copyDirectory( scriptSource, scriptDest );

    // User KWin config will be set up by post-install script
    logInfo( "KWin configuration deployed" );
}

void StrataraShellModule::setupGRUBTheme( const QString& rootMountPoint )
{
    logInfo( "Setting up GRUB theme..." );

    QString themeDir = rootMountPoint + "/usr/share/grub/themes/stratara";
    QDir().mkpath( themeDir );

    // Create theme.txt
    QString themeFile = themeDir + "/theme.txt";
    QFile file( themeFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream out( &file );
        out << "# Stratara GRUB Theme\n";
        out << "# Generated by Calamares stratara-shell module\n\n";
        out << "title-text: \"Stratara OS\"\n";
        out << "title-font: \"Noto Sans Bold 24\"\n";
        out << "title-color: \"#00aaff\"\n\n";
        out << "message-font: \"Noto Sans 16\"\n";
        out << "message-color: \"#ffffff\"\n\n";
        out << "terminal-font: \"Noto Sans Mono 14\"\n\n";
        out << "desktop-image: \"grub-bg.png\"\n";
        out << "desktop-color: \"#0a0a0c\"\n\n";
        out << "boot-menu: \"0\"\n";
        out << "menu-color-normal: \"#ffffff/#0a0a0c\"\n";
        out << "menu-color-highlight: \"#00aaff/#1a1a2e\"\n";
        out << "menu-color-border: \"#00aaff\"\n";
        out << "menu-color-timeout: \"#ff4444/#0a0a0c\"\n\n";
        out << "progress-bar: \"progress-bar.png\"\n";
        out << "progress-bar-color: \"#00aaff\"\n";
        out << "progress-bar-background: \"progress-bar-bg.png\"\n";
        file.close();
        logInfo( "Created GRUB theme: " + themeFile );
    }
    else
    {
        logError( "Failed to create GRUB theme file" );
    }

    #if 0
    // Create simple background images (SVG placeholders)
    QString bgFile = themeDir + "/grub-bg.svg";
    QFile bg( bgFile );
    if ( bg.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream out( &bg );
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<svg width=\"1920\" height=\"1080\" viewBox=\"0 0 1920 1080\" xmlns=\"http://www.w3.org/2000/svg\">\n";
        out << "  <defs>\n";
        out << "    <linearGradient id=\"bgGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">\n";
        out << "      <stop offset=\"0%\" style=\"stop-color:#0a0a0c;stop-opacity:1\" />\n";
        out << "      <stop offset=\"100%\" style=\"stop-color:#16213e;stop-opacity:1\" />\n";
        out << "    </linearGradient>\n";
        out << "    <linearGradient id=\"logoGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">\n";
        out << "      <stop offset=\"0%\" style=\"stop-color:#00aaff;stop-opacity:1\" />\n";
        out << "      <stop offset=\"100%\" style=\"stop-color:#0066cc;stop-opacity:1\" />\n";
        out << "    </linearGradient>\n";
        out << "  </defs>\n";
        out << "  <rect width=\"1920\" height=\"1080\" fill=\"url(#bgGrad)\"/>\n";
        out << "  <circle cx=\"960\" cy=\"540\" r=\"200\" fill=\"none\" stroke=\"url(#logoGrad)\" stroke-width=\"4\" opacity=\"0.3\"/>\n";
        out << "  <rect x=\"860\" y=\"440\" width=\"200\" height=\"140\" rx=\"20\" fill=\"url(#logoGrad)\"/>\n";
        out << "  <text x=\"960\" y=\"545\" font-family=\"Arial, sans-serif\" font-size=\"80\" font-weight=\"bold\" fill=\"#0a0a0c\" text-anchor=\"middle\" dominant-baseline=\"middle\">S</text>\n";
        out << "</svg>\n";
        bg.close();
    }
    #endif

    // Update GRUB config
    QString grubConfig = rootMountPoint + "/etc/default/grub";
    if ( QFile::exists( grubConfig ) )
    {
        QFile configFile( grubConfig );
        if ( configFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            QString content = configFile.readAll();
            configFile.close();

            // Update GRUB_THEME
            content.replace( QRegularExpression( "GRUB_THEME=.*" ), "GRUB_THEME=\"/usr/share/grub/themes/stratara/theme.txt\"" );
            if ( !content.contains( "GRUB_THEME=" ) )
            {
                content += "\nGRUB_THEME=\"/usr/share/grub/themes/stratara/theme.txt\"\n";
            }

            // Update GRUB_TIMEOUT
            content.replace( QRegularExpression( "GRUB_TIMEOUT=.*" ), "GRUB_TIMEOUT=5" );

            // Update GRUB_GFXMODE
            content.replace( QRegularExpression( "GRUB_GFXMODE=.*" ), "GRUB_GFXMODE=1920x1080" );

            if ( configFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
            {
                QTextStream out( &configFile );
                out << content;
                configFile.close();
                logInfo( "Updated GRUB config" );
            }
        }
    }

    // Regenerate GRUB config
    QStringList args = { "grub-mkconfig", "-o", rootMountPoint + "/boot/grub/grub.cfg" };
    if ( runCommand( "chroot", { rootMountPoint, "/usr/bin/grub-mkconfig", "-o", "/boot/grub/grub.cfg" } ) )
    {
        logInfo( "Regenerated GRUB config" );
    }
    else
    {
        logWarning( "Failed to regenerate GRUB config" );
    }
}

void StrataraShellModule::setupSecureBoot( const QString& rootMountPoint )
{
    logInfo( "Setting up Secure Boot support..." );

    // Check if shim is available
    QString shimSource = "/usr/share/shim/shimx64.efi";
    QString shimDest = rootMountPoint + "/boot/efi/EFI/BOOT/BOOTX64.EFI";

    if ( QFile::exists( shimSource ) )
    {
        QDir().mkpath( QFileInfo( shimDest ).path() );
        if ( QFile::copy( shimSource, shimDest ) )
        {
            logInfo( "Installed Secure Boot shim" );
        }
        else
        {
            logWarning( "Failed to copy Secure Boot shim" );
        }
    }
    else
    {
        logInfo( "Secure Boot shim not found, skipping" );
    }

    // Install MOK manager
    QString mokSource = "/usr/share/shim/mmx64.efi";
    QString mokDest = rootMountPoint + "/boot/efi/EFI/BOOT/mmx64.efi";

    if ( QFile::exists( mokSource ) )
    {
        QDir().mkpath( QFileInfo( mokDest ).path() );
        if ( QFile::copy( mokSource, mokDest ) )
        {
            logInfo( "Installed MOK manager" );
        }
    }
}

void StrataraShellModule::enrollXodus( const QString& rootMountPoint )
{
    logInfo( "Enrolling with Xodus..." );

    // Xodus enrollment will be done on first boot via oem-setup.sh
    // Here we just ensure the script is in place
    QString xodusctlSource = "/usr/lib/stratara/xodusctl";
    QString xodusctlDest = rootMountPoint + "/usr/lib/stratara/xodusctl";

    if ( QFile::exists( xodusctlSource ) )
    {
        if ( QFile::copy( xodusctlSource, xodusctlDest ) )
        {
            QFile::setPermissions( xodusctlDest, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup | QFile::ExeGroup );
            logInfo( "Installed xodusctl for enrollment" );
        }
    }
}

void StrataraShellModule::setupDefaultUser( const QString& rootMountPoint )
{
    logInfo( "Setting up default user..." );

    // The user is created by the users module
    // Here we just ensure Stratara-specific groups
    QStringList groups = { "wheel", "audio", "video", "network", "input", "kvm", "render" };

    for ( const QString& group : groups )
    {
        QStringList args = { "groupadd", "-f", group, "--root", rootMountPoint };
        runCommand( "chroot", { rootMountPoint, "/usr/sbin/groupadd", "-f", group } );
    }

    logInfo( "Default user groups configured" );
}

void StrataraShellModule::runPostInstallScripts( const QString& rootMountPoint )
{
    logInfo( "Running post-install scripts..." );

    QString script = "/usr/lib/stratara/post-install.sh";
    QString fullScript = rootMountPoint + script;

    if ( QFile::exists( fullScript ) )
    {
        // Make executable
        QFile::setPermissions( fullScript, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther );

        // Run in chroot
        QStringList args = { "chroot", rootMountPoint, script };
        if ( runCommand( "chroot", { rootMountPoint, "/bin/bash", script } ) )
        {
            logInfo( "Post-install script completed successfully" );
        }
        else
        {
            logWarning( "Post-install script returned non-zero exit code" );
        }
    }
    else
    {
        logWarning( "Post-install script not found: " + fullScript );
    }
}

void StrataraShellModule::copyFile( const QString& source, const QString& dest )
{
    if ( QFile::exists( source ) )
    {
        QDir().mkpath( QFileInfo( dest ).path() );
        if ( QFile::copy( source, dest ) )
        {
            logInfo( QString( "Copied %1 to %2" ).arg( source, dest ) );
        }
        else
        {
            logWarning( QString( "Failed to copy %1 to %2" ).arg( source, dest ) );
        }
    }
    else
    {
        logWarning( QString( "Source file not found: %1" ).arg( source ) );
    }
}

void StrataraShellModule::copyDirectory( const QString& source, const QString& dest )
{
    QDir sourceDir( source );
    if ( !sourceDir.exists() )
    {
        logWarning( QString( "Source directory not found: %1" ).arg( source ) );
        return;
    }

    QDir().mkpath( dest );

    QStringList files = sourceDir.entryList( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
    for ( const QString& file : files )
    {
        QString srcPath = sourceDir.filePath( file );
        QString destPath = dest + "/" + file;

        QFileInfo info( srcPath );
        if ( info.isDir() )
        {
            copyDirectory( srcPath, destPath );
        }
        else
        {
            copyFile( srcPath, destPath );
        }
    }
}

QString StrataraShellModule::readConfigValue( const QString& key, const QString& defaultValue )
{
    // Read from module configuration
    QVariantMap config = configurationMap();
    return config.value( key, defaultValue ).toString();
}

void StrataraShellModule::writeConfigValue( const QString& key, const QString& value )
{
    // Write to global storage for other modules
    Calamares::GlobalStorage* gs = Calamares::GlobalStorage::instance();
    if ( gs )
    {
        gs->insert( key, value );
    }
}

bool StrataraShellModule::runCommand( const QString& command, const QStringList& arguments, const QString& workingDir )
{
    QProcess process;
    process.setWorkingDirectory( workingDir );
    process.start( command, arguments );
    if ( !process.waitForStarted( 5000 ) )
    {
        logError( QString( "Failed to start %1: %2" ).arg( command, process.errorString() ) );
        return false;
    }

    if ( !process.waitForFinished( 60000 ) )
    {
        logError( QString( "%1 timed out" ).arg( command ) );
        return false;
    }

    if ( process.exitCode() != 0 )
    {
        QString error = process.readAllStandardError();
        logError( QString( "%1 failed (exit code %2): %3" ).arg( command, QString::number( process.exitCode() ), error.trimmed() ) );
        return false;
    }

    return true;
}

void StrataraShellModule::logInfo( const QString& message )
{
    CalamaresUtils::Logger::log( CalamaresUtils::Logger::LOG_INFO ) << "StrataraShellModule: " << message;
}

void StrataraShellModule::logWarning( const QString& message )
{
    CalamaresUtils::Logger::log( CalamaresUtils::Logger::LOG_WARNING ) << "StrataraShellModule: " << message;
}

void StrataraShellModule::logError( const QString& message )
{
    CalamaresUtils::Logger::log( CalamaresUtils::Logger::LOG_ERROR ) << "StrataraShellModule: " << message;
}

#include "main.moc"