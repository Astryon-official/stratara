#include "LauncherWindow.h"

#include <QKeyEvent>
#include <QScreen>
#include <QDebug>

namespace Stratara::UI {

LauncherWindow::LauncherWindow(QWindow *parent)
    : QQuickWindow(parent)
{
    setFlags(Qt::Window | Qt::FramelessWindowHint);
    setColor(Qt::black);

    // Start in fullscreen for TV interface
    setFullScreen(true);
}

bool LauncherWindow::isFullScreen() const
{
    return (windowState() & Qt::WindowFullScreen) != 0;
}

void LauncherWindow::setFullScreen(bool fullScreen)
{
    if (isFullScreen() == fullScreen) {
        return;
    }

    if (fullScreen) {
        showFullScreen();
    } else {
        showNormal();
    }

    emit fullScreenChanged(fullScreen);
}

void LauncherWindow::toggleFullScreen()
{
    setFullScreen(!isFullScreen());
}

void LauncherWindow::keyPressEvent(QKeyEvent *event)
{
    // Handle global shortcuts
    switch (event->key()) {
    case Qt::Key_F11:
        toggleFullScreen();
        event->accept();
        return;
    case Qt::Key_Escape:
        // Don't close on Escape - handle in QML
        break;
    default:
        break;
    }

    QQuickWindow::keyPressEvent(event);
}

void LauncherWindow::keyReleaseEvent(QKeyEvent *event)
{
    QQuickWindow::keyReleaseEvent(event);
}

} // namespace Stratara::UI