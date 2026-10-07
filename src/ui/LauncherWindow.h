#pragma once

#include <QQuickWindow>
#include <QObject>

namespace Stratara::UI {

class LauncherWindow : public QQuickWindow
{
    Q_OBJECT
    Q_PROPERTY(bool fullScreen READ isFullScreen WRITE setFullScreen NOTIFY fullScreenChanged)

public:
    explicit LauncherWindow(QWindow *parent = nullptr);
    ~LauncherWindow() override = default;

    bool isFullScreen() const;
    void setFullScreen(bool fullScreen);

    Q_INVOKABLE void toggleFullScreen();

signals:
    void fullScreenChanged(bool fullScreen);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};

} // namespace Stratara::UI