#include "UpdateDownloader.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

namespace Stratara::System {

UpdateDownloader::UpdateDownloader(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void UpdateDownloader::startDownload(const QString &url)
{
    QMutexLocker locker(&m_mutex);

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    if (m_outputFile) {
        m_outputFile->close();
        m_outputFile->deleteLater();
        m_outputFile = nullptr;
    }

    // Prepare download directory
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString downloadDir = cacheDir + "/updates";
    QDir().mkpath(downloadDir);

    QString fileName = generateFileName(url);
    m_downloadPath = downloadDir + "/" + fileName;

    m_outputFile = new QFile(m_downloadPath, this);
    if (!m_outputFile->open(QIODevice::WriteOnly)) {
        m_status = "Failed to create output file";
        emit statusChanged(m_status);
        emit downloadFinished("", false, m_status);
        return;
    }

    QNetworkRequest request{QUrl(url)};
    request.setRawHeader("User-Agent", "Stratara/0.1.0");

    m_currentReply = m_networkManager->get(request);

    connect(m_currentReply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        handleDownloadProgress(received, total);
    });

    connect(m_currentReply, &QNetworkReply::finished, this, [this]() {
        QMutexLocker locker(&m_mutex);
        handleFinished();
    });

    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, [this](QNetworkReply::NetworkError error) {
        QMutexLocker locker(&m_mutex);
        handleError(error);
    });

    m_status = "Downloading...";
    emit statusChanged(m_status);
}

void UpdateDownloader::cancelDownload()
{
    QMutexLocker locker(&m_mutex);

    if (m_currentReply) {
        m_currentReply->abort();
    }
}

void UpdateDownloader::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QMutexLocker locker(&m_mutex);

    m_bytesDownloaded = bytesReceived;
    m_bytesTotal = bytesTotal;

    if (m_outputFile && m_outputFile->isOpen()) {
        // Data is written automatically by QNetworkReply
    }

    emit progressChanged();
}

void UpdateDownloader::handleFinished()
{
    if (!m_currentReply) {
        return;
    }

    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    bool success = false;
    QString error;

    if (reply->error() == QNetworkReply::NoError) {
        if (m_outputFile && m_outputFile->isOpen()) {
            m_outputFile->flush();
            m_outputFile->close();
        }
        success = true;
        m_status = "Download complete";
    } else {
        error = reply->errorString();
        m_status = "Download failed: " + error;
        if (m_outputFile && m_outputFile->isOpen()) {
            m_outputFile->close();
            m_outputFile->remove(); // Clean up partial download
        }
    }

    emit statusChanged(m_status);
    emit downloadFinished(m_downloadPath, success, error);
    emit progressChanged();

    reply->deleteLater();
}

void UpdateDownloader::handleError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    // Error is handled in handleFinished
}

QString UpdateDownloader::generateFileName(const QString &url) const
{
    QUrl parsedUrl(url);
    QString fileName = QFileInfo(parsedUrl.path()).fileName();

    if (fileName.isEmpty()) {
        fileName = "update";
    }

    // Ensure it has an extension
    QFileInfo info(fileName);
    if (info.suffix().isEmpty()) {
        fileName += ".AppImage"; // Default assumption
    }

    return fileName;
}

qint64 UpdateDownloader::bytesDownloaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_bytesDownloaded;
}

qint64 UpdateDownloader::bytesTotal() const
{
    QMutexLocker locker(&m_mutex);
    return m_bytesTotal;
}

int UpdateDownloader::progressPercent() const
{
    QMutexLocker locker(&m_mutex);
    if (m_bytesTotal <= 0) return 0;
    return static_cast<int>((m_bytesDownloaded * 100) / m_bytesTotal);
}

QString UpdateDownloader::status() const
{
    QMutexLocker locker(&m_mutex);
    return m_status;
}

QString UpdateDownloader::downloadPath() const
{
    QMutexLocker locker(&m_mutex);
    return m_downloadPath;
}

} // namespace Stratara::System