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
    qmlRegisterType<Stratara::UI::LauncherWindow>("Stratara.UI", 1, 0, "LauncherWindow");

    // Create core objects
    auto coreApp = std::make_unique<Stratara::Core::Application>();
    auto settings = std::make_unique<Stratara::Core::Settings>();
    auto appModel = std::make_unique<Stratara::System::AppModel>(coreApp.get());
    auto iconProvider = std::make_unique<Stratara::System::IconProvider>();

    // Set up QML engine
    QQmlApplicationEngine engine;

    // Add import paths
    engine.addImportPath("qrc:/qt/qml");
    engine.addImportPath(QStringLiteral(":/qml"));

    // Expose core objects to QML
    engine.rootContext()->setContextProperty("coreApp", coreApp.get());
    engine.rootContext()->setContextProperty("settings", settings.get());
    engine.rootContext()->setContextProperty("appModel", appModel.get());

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