import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property XodusManager xodusManager

    Column {
        anchors.fill: parent
        spacing: 24
        anchors.margins: 32

        // Xodus service status
        SettingsSection {
            title: "Xodus System Service"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Xodus Service"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: xodusManager.available ? "Connected" : "Not Available"
                        font.pixelSize: 14
                        color: xodusManager.available ? "#44aa44" : "#ff4444"
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Refresh"
                    onClicked: xodusManager.refreshSystemInfo()
                }
            }

            // Version info
            Rectangle {
                visible: xodusManager.available
                Layout.fillWidth: true
                height: 80
                radius: 12
                color: "#1a1a2e"
                border.color: "#44aa44"
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 4

                    Label {
                        text: "Version: " + xodusManager.version
                        font.pixelSize: 16
                        color: "#ffffff"
                    }
                    Label {
                        text: "Status: Running"
                        font.pixelSize: 14
                        color: "#44aa44"
                    }
                }
            }
        }

        // System Information
        SettingsSection {
            title: "System Information"
            spacing: 16

            Grid {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 16
                rowSpacing: 12

                Repeater {
                    model: [
                        { label: "Hostname", value: xodusManager.hostname },
                        { label: "Kernel", value: xodusManager.kernelVersion },
                        { label: "Distribution", value: xodusManager.distroName + " " + xodusManager.distroVersion },
                        { label: "Architecture", value: xodusManager.architecture },
                        { label: "Boot Mode", value: xodusManager.bootMode },
                        { label: "Uptime", value: formatUptime(xodusManager.uptime) },
                        { label: "systemd", value: xodusManager.systemdAvailable ? "Available" : "Not Available" },
                        { label: "Xodus Version", value: xodusManager.version }
                    ]
                    delegate: Item {
                        width: parent.width / 2 - 8
                        height: 60

                        Rectangle {
                            anchors.fill: parent
                            radius: 12
                            color: "#1a1a2e"
                            border.color: "#ffffff22"
                            border.width: 1

                            Column {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 2

                                Label {
                                    text: model.label
                                    font.pixelSize: 13
                                    color: "#888888"
                                }
                                Label {
                                    text: model.value
                                    font.pixelSize: 16
                                    font.weight: Font.Medium
                                    color: "#ffffff"
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }
        }

        // Hostname management
        SettingsSection {
            title: "Hostname"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Device Hostname"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: "Current: " + xodusManager.hostname
                        font.pixelSize: 14
                        color: "#aaaaaa"
                    }
                }

                Item { Layout.fillWidth: true }
            }

            Row {
                Layout.fillWidth: true
                spacing: 12

                TextField {
                    id: hostnameField
                    Layout.fillWidth: true
                    placeholderText: "New hostname"
                    onAccepted: {
                        if (text !== "") {
                            xodusManager.setHostname(text)
                            hostnameField.text = ""
                        }
                    }
                }

                Button {
                    text: "Set Hostname"
                    enabled: hostnameField.text !== ""
                    onClicked: {
                        if (hostnameField.text !== "") {
                            xodusManager.setHostname(hostnameField.text)
                            hostnameField.text = ""
                        }
                    }
                }
            }
        }

        // Power actions
        SettingsSection {
            title: "Power Management"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "Suspend"
                    Layout.fillWidth: true
                    background: Rectangle { radius: 8; color: "#ffaa00" }
                    onClicked: xodusManager.requestPowerAction(XodusManager.Suspend)
                }

                Button {
                    text: "Reboot"
                    Layout.fillWidth: true
                    background: Rectangle { radius: 8; color: "#44aa44" }
                    onClicked: xodusManager.requestPowerAction(XodusManager.Reboot)
                }

                Button {
                    text: "Shutdown"
                    Layout.fillWidth: true
                    background: Rectangle { radius: 8; color: "#ff4444" }
                    onClicked: xodusManager.requestPowerAction(XodusManager.Shutdown)
                }
            }
        }

        // Locale settings
        SettingsSection {
            title: "Locale & Region"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Language"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: "System locale"
                        font.pixelSize: 14
                        color: "#aaaaaa"
                    }
                }

                Item { Layout.fillWidth: true }

                ComboBox {
                    id: languageCombo
                    width: 200
                    model: ["en_US", "en_GB", "de_DE", "fr_FR", "es_ES", "it_IT", "ja_JP", "zh_CN", "ko_KR"]
                    currentText: "en_US"
                    onActivated: xodusManager.setLocale(XodusManager.Language, currentText)
                }
            }

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Timezone"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: "System timezone"
                        font.pixelSize: 14
                        color: "#aaaaaa"
                    }
                }

                Item { Layout.fillWidth: true }

                ComboBox {
                    id: timezoneCombo
                    width: 250
                    model: ["UTC", "America/New_York", "America/Los_Angeles", "Europe/London", "Europe/Paris", "Europe/Berlin", "Asia/Tokyo", "Asia/Shanghai", "Australia/Sydney"]
                    currentText: "UTC"
                    onActivated: xodusManager.setTimezone(currentText)
                }
            }
        }

        // systemd units
        SettingsSection {
            title: "System Services (systemd)"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "Refresh Services"
                    onClicked: {
                        var units = xodusManager.listSystemdUnits("service")
                        console.log("Services:", units)
                    }
                }

                Button {
                    text: "List Timers"
                    onClicked: {
                        var units = xodusManager.listSystemdUnits("timer")
                        console.log("Timers:", units)
                    }
                }
            }

            Label {
                text: "Use the Diagnostics section for detailed systemd unit management"
                font.pixelSize: 14
                color: "#888888"
            }
        }

        // Xodus settings
        SettingsSection {
            title: "Xodus Configuration"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Custom Settings"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: "Key-value pairs stored in Xodus config"
                        font.pixelSize: 14
                        color: "#aaaaaa"
                    }
                }

                Item { Layout.fillWidth: true }
            }

            Row {
                Layout.fillWidth: true
                spacing: 12

                TextField {
                    id: settingKey
                    Layout.fillWidth: true
                    placeholderText: "Setting key"
                }

                TextField {
                    id: settingValue
                    Layout.fillWidth: true
                    placeholderText: "Setting value"
                }

                Button {
                    text: "Save"
                    onClicked: {
                        if (settingKey.text !== "") {
                            xodusManager.setSetting(settingKey.text, settingValue.text)
                            settingKey.text = ""
                            settingValue.text = ""
                        }
                    }
                }

                Button {
                    text: "Sync"
                    onClicked: xodusManager.syncSettings()
                }
            }
        }
    }

    function formatUptime(seconds) {
        var days = Math.floor(seconds / 86400)
        var hours = Math.floor((seconds % 86400) / 3600)
        var mins = Math.floor((seconds % 3600) / 60)
        var secs = seconds % 60
        if (days > 0) return days + "d " + hours + "h " + mins + "m"
        if (hours > 0) return hours + "h " + mins + "m"
        return mins + "m " + secs + "s"
    }
}