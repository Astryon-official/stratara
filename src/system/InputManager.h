#pragma once

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>
#include <QFile>
#include <QDebug>
#include <linux/input.h>
#include <libudev.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>

namespace Stratara::System {

class InputManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasGamepad READ hasGamepad NOTIFY gamepadStatusChanged)
    Q_PROPERTY(QString activeDevice READ activeDevice NOTIFY activeDeviceChanged)

public:
    enum class Button {
        Unknown = -1,
        // Gamepad buttons
        A = 0,           // BTN_A / BTN_SOUTH
        B = 1,           // BTN_B / BTN_EAST
        X = 2,           // BTN_X / BTN_WEST
        Y = 3,           // BTN_Y / BTN_NORTH
        LB = 4,          // BTN_TL
        RB = 5,          // BTN_TR
        LT = 6,          // BTN_TL2
        RT = 7,          // BTN_TR2
        Select = 8,      // BTN_SELECT
        Start = 9,       // BTN_START
        Guide = 10,      // BTN_MODE
        LStick = 11,     // BTN_THUMBL
        RStick = 12,     // BTN_THUMBR
        DPadUp = 13,     // BTN_DPAD_UP
        DPadDown = 14,   // BTN_DPAD_DOWN
        DPadLeft = 15,   // BTN_DPAD_LEFT
        DPadRight = 16,  // BTN_DPAD_RIGHT
        // Keyboard navigation
        Up = 100,
        Down = 101,
        Left = 102,
        Right = 103,
        Enter = 104,
        Escape = 105,
        Backspace = 106,
        Space = 107,
        Home = 108,
        End = 109,
        PageUp = 110,
        PageDown = 111,
        Tab = 112,
        // Media keys
        PlayPause = 200,
        Stop = 201,
        Next = 202,
        Previous = 203,
        VolumeUp = 204,
        VolumeDown = 205,
        Mute = 206,
    };
    Q_ENUM(Button)

    enum class Axis {
        LeftX = 0,
        LeftY = 1,
        RightX = 2,
        RightY = 3,
        LT = 4,
        RT = 5,
    };
    Q_ENUM(Axis)

    explicit InputManager(QObject *parent = nullptr);
    ~InputManager() override;

    bool hasGamepad() const;
    QString activeDevice() const;

    Q_INVOKABLE float axisValue(Axis axis) const;
    Q_INVOKABLE bool isButtonPressed(Button button) const;

    // Gamepad vibration
    Q_INVOKABLE void vibrate(float lowFrequency, float highFrequency, int durationMs);

signals:
    void gamepadStatusChanged(bool connected);
    void activeDeviceChanged(const QString &device);
    void buttonPressed(Button button);
    void buttonReleased(Button button);
    void axisChanged(Axis axis, float value);
    void keyPressed(int keyCode);
    void keyReleased(int keyCode);

private:
    void scanDevices();
    void watchDevices();
    void openDevice(const QString &devicePath, const QString &deviceName);
    void closeDevice();
    void readEvents();
    void processEvent(const input_event &ev);
    Button mapGamepadButton(uint16_t code) const;
    Button mapKeyboardKey(uint16_t code) const;
    Axis mapAxis(uint16_t code) const;

    struct GamepadState {
        bool buttons[17] = {false};
        float axes[6] = {0.0f};
        int fd = -1;
        QString path;
        QString name;
    };

    GamepadState m_gamepad;
    bool m_hasGamepad = false;
    QString m_activeDevice;

    // udev monitoring
    struct udev *m_udev = nullptr;
    struct udev_monitor *m_monitor = nullptr;
    QSocketNotifier *m_udevNotifier = nullptr;

    // Event reading
    QSocketNotifier *m_eventNotifier = nullptr;
    QTimer *m_reconnectTimer = nullptr;

    // Repeat handling
    QTimer *m_repeatTimer = nullptr;
    Button m_lastButton = Button::Unknown;
    int m_repeatDelay = 400;
    int m_repeatRate = 50;
};

} // namespace Stratara::System