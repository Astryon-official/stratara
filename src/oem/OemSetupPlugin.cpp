#include <QQmlEngine>
#include <QJSEngine>
#include "OemRunner.h"

static QObject *oemRunnerProvider(QQmlEngine *, QJSEngine *) {
    return new OemRunner();
}

void registerOemSetupTypes() {
    qmlRegisterSingletonType<OemRunner>("Stratara.OemSetup", 1, 0, "OemRunner", oemRunnerProvider);
}