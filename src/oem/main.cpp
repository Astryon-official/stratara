#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include "OemRunner.h"
#include "OemSetupPlugin.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QGuiApplication app(argc, argv);
    app.setApplicationName("Stratara OEM Setup");
    app.setApplicationDisplayName("Stratara Setup");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("Astyrion");
    app.setOrganizationDomain("astyrion.org");
    app.setWindowIcon(QIcon(":/icons/stratara.svg"));

    QQuickStyle::setStyle("Material");

    // Register OEM setup types
    registerOemSetupTypes();

    QQmlApplicationEngine engine;

    // Add import paths
    engine.addImportPath("qrc:/qt/qml");
    engine.addImportPath("qrc:/qml");

    // Register QML modules
    engine.loadFromModule("Stratara.OemSetup", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}