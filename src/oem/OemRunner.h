#ifndef OEMRUNNER_H
#define OEMRUNNER_H

#include <QObject>
#include <QVariantMap>
#include <QProcess>

class OemRunner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap setupData READ setupData WRITE setSetupData NOTIFY setupDataChanged)

public:
    explicit OemRunner(QObject *parent = nullptr);

    QVariantMap setupData() const;
    void setSetupData(const QVariantMap &data);

    Q_INVOKABLE void runSetup();

signals:
    void setupDataChanged();
    void setupStarted();
    void setupComplete();
    void setupError(const QString &message);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadOutput();
    void onReadyReadError();

private:
    QVariantMap m_setupData;
    QProcess *m_process = nullptr;
};

#endif // OEMRUNNER_H