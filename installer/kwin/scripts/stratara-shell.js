/**
 * Stratara KWin Script
 * Provides custom window management for the Stratara living-room shell
 * 
 * Features:
 * - Auto-maximize Stratara window
 * - Custom keyboard/controller navigation
 * - TV-optimized window behavior
 * - Glass/blur effect management
 */

const STRATARA_WINDOW_CLASS = "stratara";
const STRATARA_WINDOW_ROLE = "StrataraShell";

function strataraInit() {
    // Connect to window creation events
    workspace.windowAdded.connect(onWindowAdded);
    workspace.windowRemoved.connect(onWindowRemoved);
    
    // Apply rules to existing windows
    workspace.windowList().forEach(function(window) {
        if (isStrataraWindow(window)) {
            applyStrataraRules(window);
        }
    });
    
    print("Stratara KWin script loaded");
}

function onWindowAdded(window) {
    if (isStrataraWindow(window)) {
        applyStrataraRules(window);
    }
}

function onWindowRemoved(window) {
    // Cleanup if needed
}

function isStrataraWindow(window) {
    return window.windowClass === STRATARA_WINDOW_CLASS ||
           window.windowRole === STRATARA_WINDOW_ROLE ||
           window.caption === "Stratara";
}

function applyStrataraRules(window) {
    // Force fullscreen
    window.maximizeHorizontally = true;
    window.maximizeVertically = true;
    window.fullScreen = true;
    
    // No decorations
    window.noBorder = true;
    window.decorated = false;
    
    // Keep above
    window.keepAbove = true;
    
    // Skip taskbar/pager
    window.skipTaskbar = true;
    window.skipPager = true;
    window.skipSwitcher = true;
    
    // On all desktops
    window.onAllDesktops = true;
    
    // No focus stealing
    window.noFocusStealing = true;
    
    // Opacity
    window.opacityActive = 1.0;
    window.opacityInactive = 1.0;
    
    // Disable window-specific effects that interfere with shell
    window.blockGlobalShortcuts = true;
    
    print("Applied Stratara rules to window: " + window.caption);
}

// Controller/remote navigation support
function handleControllerInput(action) {
    const activeWindow = workspace.activeWindow;
    
    if (!activeWindow || !isStrataraWindow(activeWindow)) {
        return false;
    }
    
    switch (action) {
        case "up":
            // Navigate up in grid
            sendKeyEvent(activeWindow, Qt.Key_Up);
            return true;
        case "down":
            // Navigate down in grid
            sendKeyEvent(activeWindow, Qt.Key_Down);
            return true;
        case "left":
            // Navigate left in grid
            sendKeyEvent(activeWindow, Qt.Key_Left);
            return true;
        case "right":
            // Navigate right in grid
            sendKeyEvent(activeWindow, Qt.Key_Right);
            return true;
        case "select":
        case "enter":
            // Select/activate
            sendKeyEvent(activeWindow, Qt.Key_Return);
            return true;
        case "back":
            // Back/escape
            sendKeyEvent(activeWindow, Qt.Key_Escape);
            return true;
        case "home":
            // Home button - show main grid
            sendKeyEvent(activeWindow, Qt.Key_Home);
            return true;
        case "menu":
            // Menu button - show settings
            sendKeyEvent(activeWindow, Qt.Key_Menu);
            return true;
    }
    
    return false;
}

function sendKeyEvent(window, key) {
    // Send key event to the window
    const event = new QKeyEvent(QEvent.KeyPress, key, Qt.NoModifier);
    QCoreApplication.postEvent(window, event);
    
    const releaseEvent = new QKeyEvent(QEvent.KeyRelease, key, Qt.NoModifier);
    QCoreApplication.postEvent(window, releaseEvent);
}

// Glass/blur effect management
function updateGlassEffect(enabled) {
    const effect = effects.blur;
    if (effect) {
        effect.enabled = enabled;
    }
    
    const bgEffect = effects.backgroundcontrast;
    if (bgEffect) {
        bgEffect.enabled = enabled;
    }
}

// Night mode support
function setNightMode(enabled) {
    const effect = effects.colortemperature;
    if (effect) {
        effect.enabled = enabled;
        if (enabled) {
            effect.temperature = 4500;
        } else {
            effect.temperature = 6500;
        }
    }
}

// TV mode optimizations
function enableTVMode() {
    // Disable unnecessary effects for performance
    effects.desktopgrid.enabled = false;
    effects.windowview.enabled = false;
    effects.presentwindows.enabled = false;
    effects.slideback.enabled = false;
    effects.minimize.enabled = false;
    effects.maximize.enabled = false;
    
    // Enable TV-optimized effects
    effects.blur.enabled = true;
    effects.blur.strength = 32;
    effects.blur.passes = 2;
    
    effects.backgroundcontrast.enabled = true;
    effects.backgroundcontrast.intensity = 0.3;
    
    // Screen edges for remote navigation
    effects.screendedge.enabled = true;
    
    print("TV mode enabled");
}

function disableTVMode() {
    // Re-enable standard effects
    effects.desktopgrid.enabled = true;
    effects.windowview.enabled = true;
    effects.presentwindows.enabled = true;
    
    print("TV mode disabled");
}

// D-Bus interface for Stratara shell communication
const STRATARA_DBUS_SERVICE = "org.astyrion.Stratara";
const STRATARA_DBUS_PATH = "/org/astyrion/Stratara/KWin";

function registerDBusInterface() {
    // Register D-Bus interface for shell-to-KWin communication
    const bus = QDBusConnection.sessionBus();
    
    if (!bus.registerService(STRATARA_DBUS_SERVICE)) {
        print("Failed to register D-Bus service: " + bus.lastError().message());
        return false;
    }
    
    const obj = new QObject();
    obj.setObjectName("StrataraKWinInterface");
    
    // Export methods
    obj.handleControllerInput = function(action) {
        return handleControllerInput(action);
    };
    
    obj.updateGlassEffect = function(enabled) {
        updateGlassEffect(enabled);
        return true;
    };
    
    obj.setNightMode = function(enabled) {
        setNightMode(enabled);
        return true;
    };
    
    obj.enableTVMode = function() {
        enableTVMode();
        return true;
    };
    
    obj.disableTVMode = function() {
        disableTVMode();
        return true;
    };
    
    if (!bus.registerObject(STRATARA_DBUS_PATH, obj, QDBusConnection.ExportAllSlots)) {
        print("Failed to register D-Bus object: " + bus.lastError().message());
        return false;
    }
    
    print("Stratara KWin D-Bus interface registered");
    return true;
}

// Initialize on load
strataraInit();
registerDBusInterface();