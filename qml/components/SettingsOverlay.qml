import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: root
    property Settings settings
    property AppModel appModel

    signal closed()

    anchors.fill: parent
    visible: false
    focus: true

    // Modal backdrop
    Rectangle {
        id: modalBg
        anchors.fill: parent
        color: "#000000cc"
        MouseArea {
            anchors.fill: parent
            onClicked: root.close()
        }
    }

    // Settings panel
    Rectangle {
        id: panel
        width: Math.min(800, parent.width - 80)
        height: Math.min(900, parent.height - 80)
        anchors.centerIn: parent
        radius: 16
        color: "#1a1a2e"
        border.color: "#ffffff22"
        border.width: 1

        Column {
            anchors.fill: parent
            spacing: 0

            // Header
            Rectangle {
                width: parent.width
                height: 64
                radius: 16
                border.color: "#ffffff11"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Label {
                        text: "Settings"
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        color: "#ffffff"
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        id: closeBtn
                        width: 40
                        height: 40
                        radius: 20
                        background: Rectangle {
                            anchors.fill: parent
                            radius: 20
                            color: "#ffffff11"
                        }
                        contentItem: Image {
                            source: "qrc:/icons/close.svg"
                            width: 20
                            height: 20
                            color: "#ffffff"
                        }
                        onClicked: root.close()
                    }
                }
            }

            // Content area with tabs
            TabBar {
                id: tabBar
                width: parent.width
                contentWidth: parent.width

                TabButton { text: "Appearance" }
                TabButton { text: "Frame Art" }
                TabButton { text: "Weather" }
                TabButton { text: "System" }
                TabButton { text: "Apps" }
            }

            SwipeView {
                id: swipeView
                anchors.fill: parent
                currentIndex: tabBar.currentIndex
                clip: true

                // Appearance tab
                SettingsPage {
                    id: appearancePage
                    SettingsSection {
                        title: "Theme"
                        SettingsComboBox {
                            label: "Appearance"
                            model: ["Dark", "Light", "Auto"]
                            currentText: settings.theme
                            onCurrentTextChanged: settings.theme = currentText
                        }
                    }

                    SettingsSection {
                        title: "Wallpaper"
                        SettingsComboBox {
                            label: "Wallpaper"
                            model: ["Gradient 1", "Gradient 2", "Gradient 3", "Custom Image"]
                            currentIndex: settings.wallpaperId
                            onCurrentIndexChanged: settings.wallpaperId = currentIndex
                        }
                        SettingsSwitch {
                            label: "Use Custom Image"
                            checked: settings.useImageWallpaper
                            onCheckedChanged: settings.useImageWallpaper = checked
                        }
                        SettingsFilePicker {
                            label: "Custom Wallpaper"
                            enabled: settings.useImageWallpaper
                            filePath: settings.wallpaperImagePath
                            onFilePathChanged: settings.wallpaperImagePath = filePath
                        }
                    }

                    SettingsSection {
                        title: "Glass Effects"
                        SettingsSwitch {
                            label: "Frosted Glass Blur"
                            checked: settings.glassBlur
                            onCheckedChanged: settings.glassBlur = checked
                        }
                    }

                    SettingsSection {
                        title: "Layout"
                        SettingsSlider {
                            label: "Grid Columns"
                            from: 4
                            to: 7
                            value: settings.columns
                            stepSize: 1
                            onValueChanged: settings.columns = Math.round(value)
                        }
                    }
                }

                // Frame Art tab
                SettingsPage {
                    SettingsSection {
                        title: "Frame Art Source"
                        SettingsComboBox {
                            label: "Source"
                            model: ["Current Wallpaper", "Photo Folder", "Single Photo"]
                            currentIndex: {
                                switch (settings.frameSource) {
                                case "Wallpaper": return 0
                                case "Folder": return 1
                                case "Single": return 2
                                }
                            }
                            onCurrentIndexChanged: {
                                switch (currentIndex) {
                                case 0: settings.frameSource = "Wallpaper"; break
                                case 1: settings.frameSource = "Folder"; break
                                case 2: settings.frameSource = "Single"; break
                                }
                            }
                        }
                        SettingsSwitch {
                            label: "Use as Wallpaper"
                            checked: settings.useFrameArtWallpaper
                            onCheckedChanged: settings.useFrameArtWallpaper = checked
                        }
                    }

                    SettingsSection {
                        title: "Clock"
                        SettingsSwitch {
                            label: "Show Clock"
                            checked: settings.frameClock
                            onCheckedChanged: settings.frameClock = checked
                        }
                        SettingsComboBox {
                            label: "Position"
                            model: ["Bottom Left", "Bottom Center", "Bottom Right", "Center"]
                            enabled: settings.frameClock
                            currentIndex: {
                                switch (settings.frameClockPosition) {
                                case "BottomLeft": return 0
                                case "BottomCenter": return 1
                                case "BottomRight": return 2
                                case "Center": return 3
                                }
                            }
                            onCurrentIndexChanged: {
                                switch (currentIndex) {
                                case 0: settings.frameClockPosition = "BottomLeft"; break
                                case 1: settings.frameClockPosition = "BottomCenter"; break
                                case 2: settings.frameClockPosition = "BottomRight"; break
                                case 3: settings.frameClockPosition = "Center"; break
                                }
                            }
                        }
                        SettingsComboBox {
                            label: "Size"
                            model: ["Small", "Medium", "Large"]
                            enabled: settings.frameClock
                            currentIndex: {
                                switch (settings.frameClockSize) {
                                case "Small": return 0
                                case "Medium": return 1
                                case "Large": return 2
                                }
                            }
                            onCurrentIndexChanged: {
                                switch (currentIndex) {
                                case 0: settings.frameClockSize = "Small"; break
                                case 1: settings.frameClockSize = "Medium"; break
                                case 2: settings.frameClockSize = "Large"; break
                                }
                            }
                        }
                        SettingsSwitch {
                            label: "Show Date"
                            checked: settings.frameShowDate
                            enabled: settings.frameClock
                            onCheckedChanged: settings.frameShowDate = checked
                        }
                    }

                    SettingsSection {
                        title: "Motion"
                        SettingsSwitch {
                            label: "Living Painting Drift"
                            checked: settings.frameMotion
                            onCheckedChanged: settings.frameMotion = checked
                        }
                        SettingsSwitch {
                            label: "Shuffle Folder"
                            checked: settings.frameShuffle
                            enabled: settings.frameSource === "Folder"
                            onCheckedChanged: settings.frameShuffle = checked
                        }
                    }

                    SettingsSection {
                        title: "Auto-Start"
                        SettingsSlider {
                            label: "Idle Timeout (seconds)"
                            from: 0
                            to: 1800
                            value: settings.frameAutoStartSec
                            stepSize: 60
                            onValueChanged: settings.frameAutoStartSec = Math.round(value)
                        }
                    }
                }

                // Weather tab
                SettingsPage {
                    SettingsSection {
                        title: "Weather"
                        SettingsSwitch {
                            label: "Show on Home"
                            checked: settings.weatherOnHome
                            onCheckedChanged: settings.weatherOnHome = checked
                        }
                        SettingsSwitch {
                            label: "Show in Frame Art"
                            checked: settings.frameWeather
                            onCheckedChanged: settings.frameWeather = checked
                        }
                        SettingsComboBox {
                            label: "Temperature Unit"
                            model: ["Celsius", "Fahrenheit"]
                            currentIndex: settings.weatherUnit === "Celsius" ? 0 : 1
                            onCurrentIndexChanged: settings.weatherUnit = currentIndex === 0 ? "Celsius" : "Fahrenheit"
                        }
                        SettingsTextField {
                            label: "City (empty for auto)"
                            text: settings.weatherCity
                            onTextChanged: settings.weatherCity = text
                        }
                    }
                }

                // System tab
                SettingsPage {
                    SettingsSection {
                        title: "Sounds"
                        SettingsSwitch {
                            label: "Navigation Sounds"
                            checked: settings.navSounds
                            onCheckedChanged: settings.navSounds = checked
                        }
                    }

                    SettingsSection {
                        title: "Night Mode"
                        SettingsSwitch {
                            label: "Dim Frame Art at Night"
                            checked: settings.frameNightDim
                            onCheckedChanged: settings.frameNightDim = checked
                        }
                    }

                    SettingsSection {
                        title: "Media"
                        SettingsSwitch {
                            label: "Now Playing Chip"
                            checked: settings.nowPlaying
                            onCheckedChanged: settings.nowPlaying = checked
                        }
                    }
                }

                // Apps tab
                SettingsPage {
                    SettingsSection {
                        title: "Hidden Apps"
                        Label {
                            text: "Select apps to hide from the grid"
                            color: "#aaaaaa"
                            font.pixelSize: 14
                            anchors.margins: 16
                        }
                        ListView {
                            anchors.fill: parent
                            anchors.margins: 16
                            model: appModel
                            delegate: AppHideDelegate {
                                width: parent.width
                                height: 56
                                appInfo: model
                                onToggled: {
                                    // TODO: Implement hide/unhide
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    function close() {
        visible = false
        root.closed()
    }

    // Handle escape key
    Keys.onEscapePressed: close()
}

// Reusable settings components
Component {
    id: settingsSectionComponent
    Column {
        property string title
        spacing: 8

        Label {
            text: title
            font.pixelSize: 18
            font.weight: Font.Bold
            color: "#ffffff"
            anchors.margins: 16
        }

        Repeater {
            model: children
            delegate: Item { }
        }
    }
}