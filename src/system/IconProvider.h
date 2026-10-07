#pragma once

#include <QQuickImageProvider>
#include <QIcon>
#include <QPixmap>
#include <QCache>
#include <QMutex>
#include <QThread>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

namespace Stratara::System {

class IconProvider : public QQuickImageProvider
{
public:
    explicit IconProvider();
    ~IconProvider() override;

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    Q_INVOKABLE void preloadIcon(const QString &iconName);
    Q_INVOKABLE void clearCache();

private:
    QImage loadIconSync(const QString &iconName, const QSize &requestedSize);
    QString resolveIconPath(const QString &iconName) const;

    mutable QMutex m_mutex;
    QCache<QString, QImage> m_cache;
    static constexpr int MAX_CACHE_SIZE = 100; // MB
};

} // namespace Stratara::System