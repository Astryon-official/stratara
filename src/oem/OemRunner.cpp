#include "OemRunner.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QIODevice>

OemRunner::OemRunner(QObject *parent) : QObject(parent) {}

QVariantMap OemRunner::setupData() const {
    return m_setupData;
}

void OemRunner::setSetupData(const QVariantMap &data) {
    if (m_setupData != data) {
        m_setupData = data;
        emit setupDataChanged();
    }
}

void OemRunner::runSetup() {
    m_process = new QProcess(this);
    connect(m_process, &QProcess::finished, this, &OemRunner::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &OemRunner::onProcessError);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &OemRunner::onReadyReadOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &OemRunner::onReadyReadError);

    // Write setup data to a temporary JSON file
    QString tempFile = QDir::tempPath() + "/stratara-oem-setup-data.json";
    QFile file(tempFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(QJsonObject::fromVariantMap(m_setupData));
        file.write(doc.toJson());
        file.close();
    }

    // Run the OEM setup script with the data file
    QString scriptPath = "/usr/lib/stratara/oem-setup.sh";
    if (!QFile::exists(scriptPath)) {
        scriptPath = QCoreApplication::applicationDirPath() + "/../lib/stratara/oem-setup.sh";
    }

    QStringList args;
    args << "--data-file" << tempFile;

    m_process->start("pkexec", QStringList() << scriptPath << args);
    if (!m_process->waitForStarted(5000)) {
        emit setupError("Failed to start OEM setup (pkexec not available or script missing)");
        return;
    }

    emit setupStarted();
}

void OemRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        emit setupComplete();
    } else {
        QString error = m_process->readAllStandardError();
        emit setupError(QString("Setup failed with exit code %1: %2").arg(exitCode).arg(error));
    }
    m_process->deleteLater();
    m_process = nullptr;
}

void OemRunner::onProcessError(QProcess::ProcessError error) {
    QString msg;
    switch (error) {
    case QProcess::FailedToStart: msg = "Failed to start setup process"; break;
    case QProcess::Crashed: msg = "Setup process crashed"; break;
    case QProcess::Timedout: msg = "Setup process timed out"; break;
    case QProcess::WriteError: msg = "Write error to setup process"; break;
    case QProcess::ReadError: msg = "Read error from setup process"; break;
    default: msg = "Unknown setup process error"; break;
    }
    emit setupError(msg);
}

void OemRunner::onReadyReadOutput() {
    QString output = m_process->readAllStandardOutput();
    qDebug() << "OEM Setup:" << output.trimmed();
}

void OemRunner::onReadyReadError() {
    QString error = m_process->readAllStandardError();
    qWarning() << "OEM Setup Error:" << error.trimmed();
}