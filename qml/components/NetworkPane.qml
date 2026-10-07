import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property NetworkManager networkManager

    Column {
        anchors.fill: parent
        spacing: 24
        anchors.margins: 32

        // WiFi section
        SettingsSection {
            title: "Wi-Fi"
            spacing: 16

            // WiFi toggle
            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Wi-Fi"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: networkManager.connected && networkManager.connectionType === "WiFi" ? "Connected to " + networkManager.activeSSID : "Disconnected"
                        font.pixelSize: 14
                        color: "#aaaaaa"
                    }
                }

                Item { Layout.fillWidth: true }

                SettingsSwitch {
                    checked: networkManager.wifiEnabled
                    onToggled: networkManager.setWifiEnabled(checked)
                }
            }

            // Connected network
            Rectangle {
                visible: networkManager.connected && networkManager.connectionType === "WiFi"
                Layout.fillWidth: true
                height: 80
                radius: 12
                color: "#1a1a2e"
                border.color: "#44aa44"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 16

                    Image {
                        source: "qrc:/icons/wifi-warning.svg"
                        width: 32
                        height: 32
                        color: "#44aa44"
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Label {
                            text: networkManager.activeSSID
                            font.pixelSize: 18
                            font.weight: Font.Medium
                            color: "#ffffff"
                        }
                        Label {
                            text: "Signal: " + networkManager.signalStrength + "% • IP: " + networkManager.ipAddress
                            font.pixelSize: 13
                            color: "#aaaaaa"
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Forget"
                        onClicked: networkManager.forgetNetwork(networkManager.activeSSID)
                    }
                }
                anchors.margins: 12
            }

            // Available networks
            Label {
                visible: networkManager.wifiEnabled
                text: "Available Networks"
                font.pixelSize: 18
                font.weight: Font.Medium
                color: "#ffffff"
            }

            ListView {
                visible: networkManager.wifiEnabled
                Layout.fillWidth: true
                height: 300
                model: networkManager.availableNetworks
                spacing: 8
                delegate: ItemDelegate {
                    width: parent.width
                    height: 56
                    contentItem: Row {
                        anchors.fill: parent
                        spacing: 12

                        Rectangle {
                            width: 40
                            height: 40
                            radius: 20
                            color: model.security ? "#44aa44" : "#ffaa00"
                            Image {
                                anchors.centerIn: parent
                                source: "qrc:/icons/wifi-warning.svg"
                                width: 20
                                height: 20
                                color: "#ffffff"
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Label {
                                text: model.ssid
                                font.pixelSize: 18
                                color: "#ffffff"
                            }
                            Label {
                                text: model.connected ? "Connected" : (model.security ? "Secured" : "Open")
                                font.pixelSize: 13
                                color: "#aaaaaa"
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Rectangle {
                            width: 36
                            height: 36
                            radius: 18
                            color: model.signal >= 75 ? "#44aa44" : (model.signal >= 50 ? "#ffaa00" : "#ff4444")
                            Image {
                                anchors.centerIn: parent
                                source: "qrc:/icons/wifi-warning.svg"
                                width: 16
                                height: 16
                                color: "#ffffff"
                            }
                        }
                    }
                    onClicked: {
                        if (!model.connected) {
                            // Would need a password dialog here
                            console.log("Connect to:", model.ssid)
                        }
                    }
                }
            }

            Button {
                visible: networkManager.wifiEnabled
                text: "Scan Networks"
                onClicked: networkManager.scanNetworks()
            }
        }

        // Ethernet section
        SettingsSection {
            title: "Ethernet"
            spacing: 16

            Rectangle {
                visible: networkManager.connected && networkManager.connectionType === "Ethernet"
                Layout.fillWidth: true
                height: 80
                radius: 12
                color: "#1a1a2e"
                border.color: "#44aa44"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 16

                    Image {
                        source: "qrc:/icons/wifi-warning.svg"
                        width: 32
                        height: 32
                        color: "#44aa44"
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Label {
                            text: "Wired Connection"
                            font.pixelSize: 18
                            font.weight: Font.Medium
                            color: "#ffffff"
                        }
                        Label {
                            text: "IP: " + networkManager.ipAddress
                            font.pixelSize: 13
                            color: "#aaaaaa"
                        }
                    }
                }
                anchors.margins: 12
            }

            Label {
                visible: !(networkManager.connected && networkManager.connectionType === "Ethernet")
                text: "No Ethernet connection detected"
                font.pixelSize: 16
                color: "#aaaaaa"
            }
        }

        // Bluetooth section (placeholder - would use bluetoothManager)
        SettingsSection {
            title: "Bluetooth"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Bluetooth"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: "Manage paired devices and discover new ones"
                        font.pixelSize: 14
                        color: "#aaaaaa"
                    }
                }

                Item { Layout.fillWidth: true }

                SettingsSwitch {
                    checked: networkManager.bluetoothEnabled
                    onToggled: networkManager.setBluetoothEnabled(checked)
                }
            }

            Label {
                text: "Bluetooth device management is handled in a dedicated pane"
                font.pixelSize: 14
                color: "#aaaaaa"
            }
        }
    }
}