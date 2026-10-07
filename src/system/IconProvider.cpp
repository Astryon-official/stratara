#include "IconProvider.h"

#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

namespace Stratara::System {

IconProvider::IconProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_cache(MAX_CACHE_SIZE * 1024 * 1024) // Convert MB to bytes
{
}

IconProvider::~IconProvider()
{
    clearCache();
}

QImage IconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QMutexLocker locker(&m_mutex);

    // Check cache first
    QString cacheKey = id + "_" + QString::number(requestedSize.width()) + "x" + QString::number(requestedSize.height());
    if (m_cache.contains(cacheKey)) {
        QImage *cached = m_cache.object(cacheKey);
        if (cached && size) {
            *size = cached->size();
        }
        return cached ? *cached : QImage();
    }

    // Load icon
    QImage image = loadIconSync(id, requestedSize);

    if (!image.isNull()) {
        m_cache.insert(cacheKey, new QImage(image));
    }

    if (size) {
        *size = image.size();
    }
    return image;
}

void IconProvider::preloadIcon(const QString &iconName)
{
    QThreadPool::globalInstance()->start([this, iconName]() {
        QImage image = loadIconSync(iconName, QSize(256, 256));
        if (!image.isNull()) {
            QMutexLocker locker(&m_mutex);
            m_cache.insert(iconName + "_256x256", new QImage(image));
        }
    });
}

void IconProvider::clearCache()
{
    QMutexLocker locker(&m_mutex);
    // QCache automatically deletes objects when removed
    m_cache.clear();
}

QImage IconProvider::loadIconSync(const QString &iconName, const QSize &requestedSize)
{
    if (iconName.isEmpty()) {
        return QImage();
    }

    QString iconPath = resolveIconPath(iconName);

    QImage image;
    if (!iconPath.isEmpty() && QFileInfo::exists(iconPath)) {
        // Load from file
        image = QImage(iconPath);
        if (image.isNull()) {
            // Try QPixmap for SVG
            QPixmap pixmap(iconPath);
            if (!pixmap.isNull()) {
                image = pixmap.toImage();
            }
        }
    } else {
        // Try theme icon
        QIcon icon = QIcon::fromTheme(iconName);
        if (!icon.isNull()) {
            QSize targetSize = requestedSize.isValid() ? requestedSize : QSize(256, 256);
            QPixmap pixmap = icon.pixmap(targetSize);
            image = pixmap.toImage();
        }
    }

    if (image.isNull()) {
        // Return a placeholder
        QSize targetSize = requestedSize.isValid() ? requestedSize : QSize(128, 128);
        image = QImage(targetSize, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(0x2A, 0x2A, 0x2C));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(image.rect().adjusted(4, 4, -4, -4), 16, 16);
        painter.end();
    }

    // Scale to requested size if needed
    if (requestedSize.isValid() && image.size() != requestedSize) {
        image = image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return image;
}

QString IconProvider::resolveIconPath(const QString &iconName) const
{
    // If it's already a full path
    if (iconName.startsWith('/')) {
        return QFileInfo::exists(iconName) ? iconName : QString();
    }

    // Search in standard icon directories
    QStringList iconDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    iconDirs.append("/usr/share/icons");
    iconDirs.append("/usr/local/share/icons");

    QStringList themes = {"hicolor", "breeze", "Adwaita", "oxygen"};

    for (const QString &baseDir : iconDirs) {
        for (const QString &theme : themes) {
            QString themeDir = baseDir + "/icons/" + theme;
            QDir dir(themeDir);
            if (!dir.exists()) {
                continue;
            }

            // Search in scalable, then various sizes
            QStringList subdirs = {"scalable/apps", "scalable", "64x64/apps", "64x64",
                                   "48x48/apps", "48x48", "32x32/apps", "32x32",
                                   "256x256/apps", "256x256", "128x128/apps", "128x128",
                                   "apps"};

            for (const QString &subdir : subdirs) {
                QString fullPath = themeDir + "/" + subdir + "/" + iconName;
                if (QFileInfo::exists(fullPath + ".svg")) {
                    return fullPath + ".svg";
                }
                if (QFileInfo::exists(fullPath + ".png")) {
                    return fullPath + ".png";
                }
                if (QFileInfo::exists(fullPath + ".xpm")) {
                    return fullPath + ".xpm";
                }
                // Also try without extension (for theme icons)
                if (QFileInfo::exists(fullPath)) {
                    return fullPath;
                }
            }
        }
    }

    return QString();
}

} // namespace Stratara::System