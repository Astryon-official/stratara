import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

import Stratara.Core
import Stratara.System
import Stratara.UI

LauncherWindow {
    id: root
    width: 1920
    height: 1080
    visible: true
    title: "Stratara"
    color: "#000000"

    // Fullscreen on startup
    Component.onCompleted: {
        fullScreen = true
    }

    // Core application
    Application {
        id: application
    }

    // Settings
    Settings {
        id: settings
    }

    // App model
    AppModel {
        id: appModel
    }

    // Favorites store
    FavoritesStore {
        id: favoritesStore
    }

    // Update store
    UpdateStore {
        id: updateStore
    }

    // Update checker
    UpdateChecker {
        id: updateChecker
        updateStore: updateStore
    }

    // Update downloader
    UpdateDownloader {
        id: updateDownloader
    }

    // Update installer
    UpdateInstaller {
        id: updateInstaller
    }

    // Background wallpaper layer
    Rectangle {
        id: wallpaperLayer
        anchors.fill: parent
        color: "#000000"

        // Wallpaper image (when set)
        Image {
            id: wallpaperImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            source: settings.useImageWallpaper && settings.wallpaperImagePath
                    ? "file://" + settings.wallpaperImagePath
                    : ""
            visible: settings.useImageWallpaper && settings.wallpaperImagePath !== ""
            opacity: 0.6
        }

        // Gradient fallback
        GradientBackground {
            id: gradientBackground
            anchors.fill: parent
            visible: !wallpaperImage.visible
        }
    }

    // Frosted glass backdrop (for glass blur effect)
    Item {
        id: glassBackdrop
        anchors.fill: parent
        visible: settings.glassBlur
        layer.enabled: true
        layer.effect: FastBlur {
            radius: 32
            samples: 16
        }
    }

    // Main content
    Column {
        id: mainContent
        anchors.fill: parent
        spacing: 0

        // Top bar
        TopBar {
            id: topBar
            width: parent.width
            height: 80
            glassBlur: settings.glassBlur
            glassBackdrop: glassBackdrop
            networkManager: networkManager
            mediaManager: mediaManager
        }

        // Center content area
        Item {
            id: contentArea
            width: parent.width
            Layout.fillHeight: true

            // App grid/dock
            LauncherContent {
                id: launcherContent
                anchors.fill: parent
                appModel: appModel
                settings: settings
                columns: settings.columns
            }
        }
    }

    // Settings overlay
    SettingsOverlay {
        id: settingsOverlay
        anchors.fill: parent
        visible: false
        settings: settings
        appModel: appModel
        favoritesStore: favoritesStore
        updateStore: updateStore
        updateChecker: updateChecker
        updateDownloader: updateDownloader
        updateInstaller: updateInstaller
        networkManager: networkManager
        bluetoothManager: bluetoothManager
        powerManager: powerManager
        mediaManager: mediaManager
        onClosed: {
            visible = false
        }
    }

    // Keyboard navigation
    FocusScope {
        id: keyHandler
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            // Handle D-pad / controller navigation
            switch (event.key) {
            case Qt.Key_Up:
            case Qt.Key_Down:
            case Qt.Key_Left:
            case Qt.Key_Right:
            case Qt.Key_Return:
            case Qt.Key_Enter:
            case Qt.Key_Space:
            case Qt.Key_Back:
            case Qt.Key_Escape:
            case Qt.Key_Menu:
                // Let focused item handle it
                event.accepted = false
                break
            case Qt.Key_F1:
                // Toggle settings
                settingsOverlay.visible = !settingsOverlay.visible
                if (settingsOverlay.visible) {
                    settingsOverlay.forceActiveFocus()
                }
                event.accepted = true
                break
            }
        }
    }
}

// Gradient background component
Component {
    id: gradientBackgroundComponent
    Rectangle {
        id: gradientBg
        property string preset: "dark"

        gradient: Gradient {
            GradientStop { position: 0.0; color: preset === "dark" ? "#0a0a0c" : "#f5f5f7" }
            GradientStop { position: 0.5; color: preset === "dark" ? "#1a1a2e" : "#e8e8ed" }
            GradientStop { position: 1.0; color: preset === "dark" ? "#16213e" : "#d0d0d8" }
        }
    }
}