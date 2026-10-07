import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property BluetoothManager bluetoothManager

    Column {
        anchors.fill: parent
        spacing: 24
        anchors.margins: 32

        // Bluetooth toggle
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
                        text: bluetoothManager.powered ? "Adapter: " + bluetoothManager.adapterAddress : "Disabled"
                        font.pixelSize: 14
                        color: "#aaaaaa"
                    }
                }

                Item { Layout.fillWidth: true }

                SettingsSwitch {
                    checked: bluetoothManager.powered
                    onToggled: bluetoothManager.setPowered(checked)
                }
            }
        }

        // Paired devices
        SettingsSection {
            title: "Paired Devices"
            spacing: 16

            ListView {
                Layout.fillWidth: true
                height: 300
                model: bluetoothManager.pairedDevices
                spacing: 8
                delegate: ItemDelegate {
                    width: parent.width
                    height: 72
                    contentItem: Row {
                        anchors.fill: parent
                        spacing: 16

                        Rectangle {
                            width: 48
                            height: 48
                            radius: 24
                            color: model.connected ? "#44aa44" : "#2a2a2c"
                            border.color: "#ffffff22"
                            border.width: 1
                            Image {
                                anchors.centerIn: parent
                                source: "qrc:/icons/wifi-warning.svg"
                                width: 24
                                height: 24
                                color: model.connected ? "#ffffff" : "#888888"
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Label {
                                text: model.alias !== "" ? model.alias : (model.name !== "" ? model.name : model.address)
                                font.pixelSize: 18
                                font.weight: Font.Medium
                                color: "#ffffff"
                            }
                            Label {
                                text: model.address
                                font.pixelSize: 13
                                color: "#888888"
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            Button {
                                visible: !model.connected
                                text: "Connect"
                                onClicked: bluetoothManager.connectDevice(model.address)
                            }

                            Button {
                                visible: model.connected
                                text: "Disconnect"
                                onClicked: bluetoothManager.disconnectDevice(model.address)
                            }

                            Button {
                                text: model.trusted ? "Trusted" : "Trust"
                                onClicked: bluetoothManager.setTrusted(model.address, !model.trusted)
                            }

                            Button {
                                text: "Unpair"
                                onClicked: bluetoothManager.unpairDevice(model.address)
                            }
                        }
                    }
                }
            }
        }

        // Available devices
        SettingsSection {
            title: "Available Devices"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "Available Devices"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: bluetoothManager.discovering ? "Scanning..." : "Tap to scan for devices"
                        font.pixelSize: 14
                        color: "#aaaaaa"
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: bluetoothManager.discovering ? "Stop Scan" : "Scan"
                    onClicked: {
                        if (bluetoothManager.discovering) {
                            bluetoothManager.stopDiscovery()
                        } else {
                            bluetoothManager.startDiscovery()
                        }
                    }
                }
            }

            ListView {
                Layout.fillWidth: true
                height: 300
                model: bluetoothManager.devices
                spacing: 8
                delegate: ItemDelegate {
                    width: parent.width
                    height: 72
                    contentItem: Row {
                        anchors.fill: parent
                        spacing: 16

                        Rectangle {
                            width: 48
                            height: 48
                            radius: 24
                            color: "#2a2a2c"
                            border.color: "#ffffff22"
                            border.width: 1
                            Image {
                                anchors.centerIn: parent
                                source: "qrc:/icons/wifi-warning.svg"
                                width: 24
                                height: 24
                                color: "#888888"
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Label {
                                text: model.alias !== "" ? model.alias : (model.name !== "" ? model.name : model.address)
                                font.pixelSize: 18
                                font.weight: Font.Medium
                                color: "#ffffff"
                            }
                            Label {
                                text: model.address
                                font.pixelSize: 13
                                color: "#888888"
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            Button {
                                visible: model.paired
                                text: "Unpair"
                                onClicked: bluetoothManager.unpairDevice(model.address)
                            }

                            Button {
                                visible: !model.paired
                                text: "Pair"
                                onClicked: bluetoothManager.pairDevice(model.address)
                            }

                            Button {
                                visible: model.paired && !model.connected
                                text: "Connect"
                                onClicked: bluetoothManager.connectDevice(model.address)
                            }

                            Button {
                                visible: model.paired && model.connected
                                text: "Disconnect"
                                onClicked: bluetoothManager.disconnectDevice(model.address)
                            }
                        }
                    }
                }
            }
        }
    }
}