#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "XodusService.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("xodus");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("Astryon");

    QCommandLineParser parser;
    parser.setApplicationDescription("Xodus - Stratara System Integration Service");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption systemdOption("systemd", "Run as systemd service (default)");
    QCommandLineOption userOption("user", "Run as user session service");
    parser.addOption(systemdOption);
    parser.addOption(userOption);

    parser.process(app);

    Stratara::Xodus::XodusService service;
    if (!service.initialize()) {
        qCritical() << "Failed to initialize Xodus service";
        return 1;
    }

    qInfo() << "Xodus service running...";
    return app.exec();
}