import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property PowerManager powerManager

    Column {
        anchors.fill: parent
        spacing: 24
        anchors.margins: 32

        // Battery status
        SettingsSection {
            title: "Battery"
            spacing: 16

            Rectangle {
                Layout.fillWidth: true
                height: 120
                radius: 16
                color: powerManager.batteryState === "Discharging" ? "#2a1a1a" : "#1a2a1a"
                border.color: powerManager.batteryState === "Discharging" ? "#ff6666" : "#44aa44"
                border.width: 2

                Row {
                    anchors.centerIn: parent
                    spacing: 24

                    // Battery icon with percentage
                    Item {
                        width: 80
                        height: 80

                        Rectangle {
                            anchors.centerIn: parent
                            width: 60
                            height: 36
                            radius: 6
                            border.color: "#ffffff"
                            border.width: 2
                            color: "transparent"
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: 10
                            height: 18
                            x: 36
                            radius: 2
                            color: "#ffffff"
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: 54 * (powerManager.batteryPercentage / 100)
                            height: 30
                            x: -27 + 27 * (1 - powerManager.batteryPercentage / 100)
                            radius: 3
                            color: powerManager.batteryPercentage <= 15 ? "#ff4444" : (powerManager.batteryPercentage <= 30 ? "#ffaa00" : "#44aa44")
                        }

                        Label {
                            anchors.centerIn: parent
                            text: powerManager.batteryPercentage + "%"
                            font.pixelSize: 20
                            font.weight: Font.Bold
                            color: "#ffffff"
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4

                        Label {
                            text: powerManager.onBattery ? "On Battery" : "Charging"
                            font.pixelSize: 24
                            font.weight: Font.Bold
                            color: "#ffffff"
                        }

                        Label {
                            text: powerManager.batteryState
                            font.pixelSize: 16
                            color: "#aaaaaa"
                        }

                        Label {
                            text: powerManager.batteryTimeRemaining >= 0
                                ? "Time remaining: " + formatTime(powerManager.batteryTimeRemaining)
                                : "Calculating..."
                            font.pixelSize: 14
                            color: "#888888"
                        }
                    }
                }
            }

            // Lid status
            Rectangle {
                Layout.fillWidth: true
                height: 60
                radius: 12
                color: "#1a1a2e"
                border.color: "#ffffff22"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 16
                    anchors.margins: 16

                    Image {
                        source: "qrc:/icons/wifi-warning.svg"
                        width: 28
                        height: 28
                        color: powerManager.lidClosed ? "#ff4444" : "#44aa44"
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        Label {
                            text: "Lid Status"
                            font.pixelSize: 18
                            font.weight: Font.Medium
                            color: "#ffffff"
                        }
                        Label {
                            text: powerManager.lidClosed ? "Closed" : "Open"
                            font.pixelSize: 14
                            color: "#aaaaaa"
                        }
                    }
                }
            }
        }

        // Display brightness
        SettingsSection {
            title: "Display Brightness"
            spacing: 16

            Slider {
                id: brightnessSlider
                Layout.fillWidth: true
                from: 0
                to: powerManager.maxBrightness > 0 ? powerManager.maxBrightness : 100
                value: powerManager.brightness
                stepSize: 1
                onValueChanged: {
                    if (!pressed) powerManager.setBrightness(value)
                }
            }

            Row {
                Layout.fillWidth: true
                spacing: 16
                Label {
                    text: "Brightness: " + powerManager.brightness + "%"
                    font.pixelSize: 16
                    color: "#ffffff"
                }
            }
        }

        // Power actions
        SettingsSection {
            title: "Power Actions"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    Layout.fillWidth: true
                    text: "Suspend"
                    background: Rectangle {
                        radius: 12
                        color: "#2a2a2c"
                        border.color: "#ffffff22"
                        border.width: 1
                    }
                    onClicked: powerManager.suspend()
                }

                Button {
                    Layout.fillWidth: true
                    text: "Hibernate"
                    background: Rectangle {
                        radius: 12
                        color: "#2a2a2c"
                        border.color: "#ffffff22"
                        border.width: 1
                    }
                    onClicked: powerManager.hibernate()
                }
            }

            Row {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    Layout.fillWidth: true
                    text: "Reboot"
                    background: Rectangle {
                        radius: 12
                        color: "#2a1a1a"
                        border.color: "#ff6666"
                        border.width: 1
                    }
                    onClicked: powerManager.reboot()
                }

                Button {
                    Layout.fillWidth: true
                    text: "Shutdown"
                    background: Rectangle {
                        radius: 12
                        color: "#2a1a1a"
                        border.color: "#ff6666"
                        border.width: 1
                    }
                    onClicked: powerManager.shutdown()
                }
            }
        }
    }

    function formatTime(seconds) {
        var hours = Math.floor(seconds / 3600)
        var minutes = Math.floor((seconds % 3600) / 60)
        if (hours > 0) {
            return hours + "h " + minutes + "m"
        } else {
            return minutes + "m"
        }
    }
}