import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property NotificationManager notificationManager

    Column {
        anchors.fill: parent
        spacing: 24
        anchors.margins: 32

        // Notification status
        SettingsSection {
            title: "Notifications"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Notification Daemon"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: notificationManager.available ? "Connected" : "Not Available"
                        font.pixelSize: 14
                        color: notificationManager.available ? "#44aa44" : "#ff4444"
                    }
                }

                Item { Layout.fillWidth: true }

                SettingsSwitch {
                    checked: notificationManager.doNotDisturb
                    onToggled: notificationManager.setDoNotDisturb(checked)
                }
            }

            Label {
                text: notificationManager.doNotDisturb ? "Do Not Disturb is enabled - notifications are silenced" : ""
                font.pixelSize: 14
                color: "#ffaa00"
            }
        }

        // Test notification buttons
        SettingsSection {
            title: "Test Notifications"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "Low Priority"
                    Layout.fillWidth: true
                    onClicked: {
                        notificationManager.sendNotification(
                            "Stratara", "Test Low Priority",
                            "This is a low priority notification",
                            "", 5000, NotificationManager.Low
                        )
                    }
                }

                Button {
                    text: "Normal"
                    Layout.fillWidth: true
                    onClicked: {
                        notificationManager.sendNotification(
                            "Stratara", "Test Normal",
                            "This is a normal priority notification",
                            "", 5000, NotificationManager.Normal
                        )
                    }
                }

                Button {
                    text: "Critical"
                    Layout.fillWidth: true
                    onClicked: {
                        notificationManager.sendNotification(
                            "Stratara", "Test Critical",
                            "This is a critical notification!",
                            "", 0, NotificationManager.Critical
                        )
                    }
                }
            }

            Row {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "With Actions"
                    Layout.fillWidth: true
                    onClicked: {
                        var hints = {
                            "actions": [
                                ["accept", "Accept"],
                                ["dismiss", "Dismiss"]
                            ]
                        }
                        notificationManager.sendNotification(
                            "Stratara", "Action Test",
                            "This notification has action buttons",
                            "", 10000, NotificationManager.Normal, "", hints
                        )
                    }
                }

                Button {
                    text: "Clear History"
                    Layout.fillWidth: true
                    onClicked: notificationManager.clearHistory()
                }
            }
        }

        // Notification history
        SettingsSection {
            title: "Recent Notifications"
            spacing: 16

            ListView {
                Layout.fillWidth: true
                height: 400
                model: notificationManager.history
                spacing: 8
                delegate: ItemDelegate {
                    width: parent.width
                    height: 90
                    contentItem: Rectangle {
                        anchors.fill: parent
                        radius: 12
                        color: "#1a1a2e"
                        border.color: "#ffffff22"
                        border.width: 1

                        Column {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 4

                            Row {
                                spacing: 12
                                Label {
                                    text: model.appName
                                    font.pixelSize: 16
                                    font.weight: Font.Medium
                                    color: "#ffffff"
                                }
                                Label {
                                    text: model.timestamp
                                    font.pixelSize: 12
                                    color: "#888888"
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: urgencyText(model.urgency)
                                    font.pixelSize: 12
                                    color: urgencyColor(model.urgency)
                                }
                            }

                            Label {
                                text: model.summary
                                font.pixelSize: 18
                                font.weight: Font.Medium
                                color: "#ffffff"
                                elide: Text.ElideRight
                            }

                            Label {
                                text: model.body
                                font.pixelSize: 14
                                color: "#aaaaaa"
                                elide: Text.ElideRight
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                // Could show details or actions
                            }
                        }
                    }
                }
            }

            Label {
                visible: notificationManager.history.length === 0
                text: "No notifications yet"
                font.pixelSize: 16
                color: "#aaaaaa"
            }
        }
    }

    function urgencyText(urgency) {
        switch (urgency) {
            case 0: return "Low"
            case 1: return "Normal"
            case 2: return "Critical"
            default: return "Unknown"
        }
    }

    function urgencyColor(urgency) {
        switch (urgency) {
            case 0: return "#888888"
            case 1: return "#ffffff"
            case 2: return "#ff4444"
            default: return "#aaaaaa"
        }
    }
}