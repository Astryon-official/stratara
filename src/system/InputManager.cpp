#include "InputManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThread>
#include <libudev.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cmath>

namespace Stratara::System {

InputManager::InputManager(QObject *parent)
    : QObject(parent)
{
    // Initialize udev
    m_udev = udev_new();
    if (!m_udev) {
        qWarning() << "Failed to initialize udev";
    } else {
        // Monitor input subsystem
        m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
        if (m_monitor) {
            udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "input", nullptr);
            udev_monitor_enable_receiving(m_monitor);

            int fd = udev_monitor_get_fd(m_monitor);
            if (fd >= 0) {
                m_udevNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
                connect(m_udevNotifier, &QSocketNotifier::activated, this, &InputManager::watchDevices);
            }
        }

        // Initial scan
        scanDevices();
    }

    // Reconnect timer for gamepad
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(2000);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &InputManager::scanDevices);

    // Button repeat timer
    m_repeatTimer = new QTimer(this);
    m_repeatTimer->setSingleShot(true);
    m_repeatTimer->setInterval(m_repeatRate);
    connect(m_repeatTimer, &QTimer::timeout, this, [this]() {
        if (m_lastButton != Button::Unknown) {
            emit buttonPressed(m_lastButton);
            m_repeatTimer->start(m_repeatRate);
        }
    });
}

InputManager::~InputManager()
{
    closeDevice();

    if (m_udevNotifier) {
        m_udevNotifier->setEnabled(false);
        delete m_udevNotifier;
    }
    if (m_monitor) {
        udev_monitor_unref(m_monitor);
    }
    if (m_udev) {
        udev_unref(m_udev);
    }
}

bool InputManager::hasGamepad() const
{
    return m_hasGamepad;
}

QString InputManager::activeDevice() const
{
    return m_activeDevice;
}

float InputManager::axisValue(Axis axis) const
{
    int idx = static_cast<int>(axis);
    if (idx >= 0 && idx < 6) {
        return m_gamepad.axes[idx];
    }
    return 0.0f;
}

bool InputManager::isButtonPressed(Button button) const
{
    int idx = static_cast<int>(button);
    if (idx >= 0 && idx < 17) {
        return m_gamepad.buttons[idx];
    }
    return false;
}

void InputManager::vibrate(float lowFrequency, float highFrequency, int durationMs)
{
    if (m_gamepad.fd < 0) return;

    // Use FF (force feedback) if available
    struct ff_effect effect;
    memset(&effect, 0, sizeof(effect));
    effect.type = FF_RUMBLE;
    effect.id = -1;
    effect.u.rumble.strong_magnitude = static_cast<uint16_t>(highFrequency * 65535);
    effect.u.rumble.weak_magnitude = static_cast<uint16_t>(lowFrequency * 65535);
    effect.replay.length = durationMs;
    effect.replay.delay = 0;

    int effectId = ioctl(m_gamepad.fd, EVIOCSFF, &effect);
    if (effectId >= 0) {
        struct input_event play;
        play.type = EV_FF;
        play.code = effectId;
        play.value = 1;
        write(m_gamepad.fd, &play, sizeof(play));

        // Clean up after duration
        QTimer::singleShot(durationMs, this, [this, effectId]() {
            if (m_gamepad.fd >= 0) {
                ioctl(m_gamepad.fd, EVIOCRMFF, effectId);
            }
        });
    }
}

void InputManager::scanDevices()
{
    if (!m_udev) return;

    struct udev_enumerate *enumerate = udev_enumerate_new(m_udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *device = udev_device_new_from_syspath(m_udev, path);

        if (device) {
            const char *devnode = udev_device_get_devnode(device);
            const char *name = udev_device_get_property_value(device, "ID_INPUT_JOYSTICK") ? "gamepad" :
                               udev_device_get_property_value(device, "ID_INPUT_KEYBOARD") ? "keyboard" :
                               udev_device_get_property_value(device, "ID_INPUT_MOUSE") ? "mouse" : "unknown";

            if (devnode && (strcmp(name, "gamepad") == 0 || strcmp(name, "keyboard") == 0)) {
                if (m_gamepad.fd < 0 && strcmp(name, "gamepad") == 0) {
                    const char *deviceName = udev_device_get_sysattr_value(device, "name");
                    openDevice(devnode, deviceName ? deviceName : "Unknown Gamepad");
                }
            }
            udev_device_unref(device);
        }
    }
    udev_enumerate_unref(enumerate);
}

void InputManager::watchDevices()
{
    struct udev_device *device = udev_monitor_receive_device(m_monitor);
    if (device) {
        const char *action = udev_device_get_action(device);
        const char *devnode = udev_device_get_devnode(device);
        const char *subsystem = udev_device_get_subsystem(device);

        if (devnode && subsystem && strcmp(subsystem, "input") == 0) {
            if (action && (strcmp(action, "add") == 0 || strcmp(action, "remove") == 0)) {
                // Rescan on device changes
                if (m_gamepad.fd < 0 || strcmp(action, "remove") == 0) {
                    m_reconnectTimer->start();
                }
            }
        }
        udev_device_unref(device);
    }
}

void InputManager::openDevice(const QString &devicePath, const QString &deviceName)
{
    if (m_gamepad.fd >= 0) {
        closeDevice();
    }

    int fd = open(devicePath.toUtf8().constData(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        qWarning() << "Failed to open input device:" << devicePath << strerror(errno);
        return;
    }

    // Check capabilities
    unsigned long evbit[EV_MAX / 8 / sizeof(unsigned long) + 1];
    if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0) {
        close(fd);
        return;
    }

    // Check for gamepad capabilities (buttons + axes)
    bool hasButtons = (evbit[EV_KEY / 8 / sizeof(unsigned long)] & (1ULL << (EV_KEY % 64))) != 0;
    bool hasAbs = (evbit[EV_ABS / 8 / sizeof(unsigned long)] & (1ULL << (EV_ABS % 64))) != 0;

    if (!hasButtons || !hasAbs) {
        // Not a gamepad, might be keyboard - still open for keyboard input
        if (hasButtons) {
            // Keep keyboard for navigation
        } else {
            close(fd);
            return;
        }
    }

    // Get device name
    char name[256] = "Unknown";
    ioctl(fd, EVIOCGNAME(sizeof(name)), name);

    m_gamepad.fd = fd;
    m_gamepad.path = devicePath;
    m_gamepad.name = deviceName.isEmpty() ? QString::fromLatin1(name) : deviceName;

    m_eventNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_eventNotifier, &QSocketNotifier::activated, this, &InputManager::readEvents);

    if (hasAbs) {
        m_hasGamepad = true;
        m_activeDevice = m_gamepad.name;
        emit gamepadStatusChanged(true);
        emit activeDeviceChanged(m_activeDevice);
        qInfo() << "Gamepad connected:" << m_activeDevice << "at" << devicePath;
    } else {
        qInfo() << "Keyboard/input device connected:" << m_gamepad.name << "at" << devicePath;
    }
}

void InputManager::closeDevice()
{
    if (m_eventNotifier) {
        m_eventNotifier->setEnabled(false);
        delete m_eventNotifier;
        m_eventNotifier = nullptr;
    }

    if (m_gamepad.fd >= 0) {
        // Clear any active effects
        for (int i = 0; i < 10; ++i) {
            ioctl(m_gamepad.fd, EVIOCRMFF, i);
        }
        close(m_gamepad.fd);
        m_gamepad.fd = -1;
    }

    if (m_hasGamepad) {
        m_hasGamepad = false;
        m_activeDevice.clear();
        emit gamepadStatusChanged(false);
        emit activeDeviceChanged(QString());
        qInfo() << "Gamepad disconnected";
    }

    // Reset state
    memset(m_gamepad.buttons, 0, sizeof(m_gamepad.buttons));
    memset(m_gamepad.axes, 0, sizeof(m_gamepad.axes));
}

void InputManager::readEvents()
{
    if (m_gamepad.fd < 0) return;

    input_event ev;
    while (read(m_gamepad.fd, &ev, sizeof(ev)) == sizeof(ev)) {
        processEvent(ev);
    }
}

void InputManager::processEvent(const input_event &ev)
{
    if (ev.type == EV_KEY) {
        // Button/key event
        Button button = Button::Unknown;

        if (ev.code >= BTN_GAMEPAD && ev.code < 0x200) {
            button = mapGamepadButton(ev.code);
        } else {
            button = mapKeyboardKey(ev.code);
        }

        if (button != Button::Unknown) {
            int idx = static_cast<int>(button);
            bool pressed = ev.value != 0;

            if (idx >= 0 && idx < 17) {
                if (pressed != m_gamepad.buttons[idx]) {
                    m_gamepad.buttons[idx] = pressed;
                    if (pressed) {
                        emit buttonPressed(button);
                        m_lastButton = button;
                        m_repeatTimer->start(m_repeatDelay);
                    } else {
                        emit buttonReleased(button);
                        if (m_lastButton == button) {
                            m_repeatTimer->stop();
                            m_lastButton = Button::Unknown;
                        }
                    }
                }
            } else {
                // Keyboard navigation keys
                if (pressed) {
                    emit keyPressed(static_cast<int>(button));
                    m_lastButton = button;
                    m_repeatTimer->start(m_repeatDelay);
                } else {
                    emit keyReleased(static_cast<int>(button));
                    if (m_lastButton == button) {
                        m_repeatTimer->stop();
                        m_lastButton = Button::Unknown;
                    }
                }
            }
        }
    } else if (ev.type == EV_ABS) {
        // Axis event
        Axis axis = mapAxis(ev.code);
        int idx = static_cast<int>(axis);
        if (idx >= 0 && idx < 6) {
            // Normalize axis value (-32768 to 32767 -> -1.0 to 1.0)
            float value = 0.0f;
            if (ev.code == ABS_X || ev.code == ABS_Y || ev.code == ABS_RX || ev.code == ABS_RY) {
                value = static_cast<float>(ev.value) / 32767.0f;
            } else if (ev.code == ABS_Z || ev.code == ABS_RZ) {
                value = static_cast<float>(ev.value) / 32767.0f;
            } else if (ev.code == ABS_HAT0X) {
                value = static_cast<float>(ev.value); // -1, 0, 1
            } else if (ev.code == ABS_HAT0Y) {
                value = static_cast<float>(ev.value); // -1, 0, 1
            }

            // Apply deadzone
            if (std::abs(value) < 0.1f) value = 0.0f;

            if (value != m_gamepad.axes[idx]) {
                m_gamepad.axes[idx] = value;
                emit axisChanged(axis, value);
            }
        }
    }
}

InputManager::Button InputManager::mapGamepadButton(uint16_t code) const
{
    // BTN_A/BTN_SOUTH, BTN_B/BTN_EAST, BTN_X/BTN_WEST, BTN_Y/BTN_NORTH are aliases
    if (code == BTN_A || code == BTN_SOUTH) return Button::A;
    if (code == BTN_B || code == BTN_EAST) return Button::B;
    if (code == BTN_X || code == BTN_WEST) return Button::X;
    if (code == BTN_Y || code == BTN_NORTH) return Button::Y;
    if (code == BTN_TL) return Button::LB;
    if (code == BTN_TR) return Button::RB;
    if (code == BTN_TL2) return Button::LT;
    if (code == BTN_TR2) return Button::RT;
    if (code == BTN_SELECT) return Button::Select;
    if (code == BTN_START) return Button::Start;
    if (code == BTN_MODE) return Button::Guide;
    if (code == BTN_THUMBL) return Button::LStick;
    if (code == BTN_THUMBR) return Button::RStick;
    if (code == BTN_DPAD_UP) return Button::DPadUp;
    if (code == BTN_DPAD_DOWN) return Button::DPadDown;
    if (code == BTN_DPAD_LEFT) return Button::DPadLeft;
    if (code == BTN_DPAD_RIGHT) return Button::DPadRight;
    return Button::Unknown;
}

InputManager::Button InputManager::mapKeyboardKey(uint16_t code) const
{
    switch (code) {
    case KEY_UP: return Button::Up;
    case KEY_DOWN: return Button::Down;
    case KEY_LEFT: return Button::Left;
    case KEY_RIGHT: return Button::Right;
    case KEY_ENTER: case KEY_KPENTER: return Button::Enter;
    case KEY_ESC: return Button::Escape;
    case KEY_BACKSPACE: return Button::Backspace;
    case KEY_SPACE: return Button::Space;
    case KEY_HOME: return Button::Home;
    case KEY_END: return Button::End;
    case KEY_PAGEUP: return Button::PageUp;
    case KEY_PAGEDOWN: return Button::PageDown;
    case KEY_TAB: return Button::Tab;
    case KEY_PLAYPAUSE: return Button::PlayPause;
    case KEY_STOP: return Button::Stop;
    case KEY_NEXTSONG: return Button::Next;
    case KEY_PREVIOUSSONG: return Button::Previous;
    case KEY_VOLUMEUP: return Button::VolumeUp;
    case KEY_VOLUMEDOWN: return Button::VolumeDown;
    case KEY_MUTE: return Button::Mute;
    default: return Button::Unknown;
    }
}

InputManager::Axis InputManager::mapAxis(uint16_t code) const
{
    switch (code) {
    case ABS_X: return Axis::LeftX;
    case ABS_Y: return Axis::LeftY;
    case ABS_RX: return Axis::RightX;
    case ABS_RY: return Axis::RightY;
    case ABS_Z: return Axis::LT;
    case ABS_RZ: return Axis::RT;
    case ABS_HAT0X: return Axis::LeftX; // D-pad as axis
    case ABS_HAT0Y: return Axis::LeftY;
    default: return Axis::LeftX;
    }
}

} // namespace Stratara::System