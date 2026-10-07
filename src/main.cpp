#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QLoggingCategory>
#include <QDir>
#include <QStandardPaths>

#include "core/Application.h"
#include "core/Settings.h"
#include "system/AppModel.h"
#include "system/IconProvider.h"
#include "system/FavoritesStore.h"
#include "system/AppListCache.h"
#include "system/UpdateStore.h"
#include "system/UpdateChecker.h"
#include "system/UpdateDownloader.h"
#include "system/UpdateInstaller.h"
#include "system/UpdateResult.h"
#include "ui/LauncherWindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("Stratara");
    app.setApplicationDisplayName("Stratara");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("Astryon");
    app.setOrganizationDomain("astyrion.org");
    app.setWindowIcon(QIcon(":/icons/stratara.svg"));

    // Register QML types
    qmlRegisterType<Stratara::Core::Application>("Stratara.Core", 1, 0, "Application");
    qmlRegisterType<Stratara::Core::Settings>("Stratara.Core", 1, 0, "Settings");
    qmlRegisterType<Stratara::System::AppModel>("Stratara.System", 1, 0, "AppModel");
    qmlRegisterType<Stratara::System::IconProvider>("Stratara.System", 1, 0, "IconProvider");
    qmlRegisterType<Stratara::System::FavoritesStore>("Stratara.System", 1, 0, "FavoritesStore");
    qmlRegisterType<Stratara::System::AppListCache>("Stratara.System", 1, 0, "AppListCache");
    qmlRegisterType<Stratara::System::UpdateStore>("Stratara.System", 1, 0, "UpdateStore");
    qmlRegisterType<Stratara::System::UpdateChecker>("Stratara.System", 1, 0, "UpdateChecker");
    qmlRegisterType<Stratara::System::UpdateDownloader>("Stratara.System", 1, 0, "UpdateDownloader");
    qmlRegisterType<Stratara::System::UpdateInstaller>("Stratara.System", 1, 0, "UpdateInstaller");
    qmlRegisterType<Stratara::UI::LauncherWindow>("Stratara.UI", 1, 0, "LauncherWindow");

    // Register metatypes for signals/slots
    qRegisterMetaType<Stratara::System::UpdateResult>("UpdateResult");
    qRegisterMetaType<Stratara::System::PendingUpdate>("PendingUpdate");
    qRegisterMetaType<Stratara::System::InstallResult>("InstallResult");

    // Create core objects
    auto coreApp = std::make_unique<Stratara::Core::Application>();
    auto settings = std::make_unique<Stratara::Core::Settings>();
    auto appModel = std::make_unique<Stratara::System::AppModel>(coreApp.get());
    auto iconProvider = std::make_unique<Stratara::System::IconProvider>();
    auto favoritesStore = std::make_unique<Stratara::System::FavoritesStore>();
    auto appListCache = std::make_unique<Stratara::System::AppListCache>();
    auto updateStore = std::make_unique<Stratara::System::UpdateStore>();
    auto updateChecker = std::make_unique<Stratara::System::UpdateChecker>(updateStore.get());
    auto updateDownloader = std::make_unique<Stratara::System::UpdateDownloader>();
    auto updateInstaller = std::make_unique<Stratara::System::UpdateInstaller>();

    // Set up QML engine
    QQmlApplicationEngine engine;

    // Add import paths
    engine.addImportPath("qrc:/qt/qml");
    engine.addImportPath(QStringLiteral(":/qml"));

    // Expose core objects to QML
    engine.rootContext()->setContextProperty("coreApp", coreApp.get());
    engine.rootContext()->setContextProperty("settings", settings.get());
    engine.rootContext()->setContextProperty("appModel", appModel.get());
    engine.rootContext()->setContextProperty("favoritesStore", favoritesStore.get());
    engine.rootContext()->setContextProperty("appListCache", appListCache.get());
    engine.rootContext()->setContextProperty("updateStore", updateStore.get());
    engine.rootContext()->setContextProperty("updateChecker", updateChecker.get());
    engine.rootContext()->setContextProperty("updateDownloader", updateDownloader.get());
    engine.rootContext()->setContextProperty("updateInstaller", updateInstaller.get());

    // Add image provider for app icons
    engine.addImageProvider("appicons", iconProvider.get());

    // Load main QML
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    engine.load(url);

    // Initialize core application
    coreApp->initialize();
    appModel->refresh();

    return app.exec();
}