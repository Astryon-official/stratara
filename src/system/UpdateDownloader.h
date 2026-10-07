#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QMutex>

namespace Stratara::System {

class UpdateDownloader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 bytesDownloaded READ bytesDownloaded NOTIFY progressChanged)
    Q_PROPERTY(qint64 bytesTotal READ bytesTotal NOTIFY progressChanged)
    Q_PROPERTY(int progressPercent READ progressPercent NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath CONSTANT)

public:
    explicit UpdateDownloader(QObject *parent = nullptr);
    ~UpdateDownloader() override = default;

    Q_INVOKABLE void startDownload(const QString &url);
    Q_INVOKABLE void cancelDownload();

    qint64 bytesDownloaded() const;
    qint64 bytesTotal() const;
    int progressPercent() const;
    QString status() const;
    QString downloadPath() const;

signals:
    void progressChanged();
    void statusChanged(const QString &status);
    void downloadFinished(const QString &filePath, bool success, const QString &error = QString());

private:
    void handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void handleFinished();
    void handleError(QNetworkReply::NetworkError error);
    QString generateFileName(const QString &url) const;

    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply = nullptr;
    QFile *m_outputFile = nullptr;
    QString m_downloadPath;
    qint64 m_bytesDownloaded = 0;
    qint64 m_bytesTotal = 0;
    QString m_status = "Idle";
    mutable QMutex m_mutex;
};

} // namespace Stratara::System