import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var setupData
    property bool canProceed: true
    property var availableNetworks: []
    property string selectedSsid: ""
    property bool isScanning: false

    signal canProceedChanged()

    // NetworkManager integration
    Connections {
        target: networkManager
        onWifiNetworksChanged: {
            root.availableNetworks = networkManager.wifiNetworks
        }
        onWifiScanComplete: {
            root.isScanning = false
        }
    }

    Component.onCompleted: {
        networkManager.requestScan()
        root.isScanning = true
    }

    Column {
        anchors.centerIn: parent
        spacing: 24
        width: parent.width

        // Title
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Network Connection"
            color: "#ffffff"
            font.pixelSize: 36
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Connect to the internet for updates and Xodus enrollment"
            color: "#888888"
            font.pixelSize: 16
        }

        // Connection type selector
        Column {
            spacing: 16
            width: Math.min(parent.width, 600)
            anchors.horizontalCenter: parent.horizontalCenter

            // Ethernet status
            Rectangle {
                width: parent.width
                height: 60
                radius: 12
                color: networkManager.ethernetConnected ? "#003322" : "#1e1e2e"
                border.color: networkManager.ethernetConnected ? "#00d4aa" : "#333333"
                border.width: 2

                Row {
                    anchors.centerIn: parent
                    spacing: 16

                    Image {
                        source: "qrc:/icons/wifi-warning.svg"
                        width: 28
                        height: 28
                        color: networkManager.ethernetConnected ? "#00d4aa" : "#666666"
                    }

                    Column {
                        Text {
                            text: "Ethernet"
                            color: "#ffffff"
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Text {
                            text: networkManager.ethernetConnected ? "Connected" : "Not connected"
                            color: networkManager.ethernetConnected ? "#00d4aa" : "#888888"
                            font.pixelSize: 13
                        }
                    }
                }
            }

            // WiFi section
            Column {
                spacing: 12
                width: parent.width

                Row {
                    spacing: 12
                    Text {
                        text: "WiFi"
                        color: "#ffffff"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    Button {
                        text: root.isScanning ? "Scanning..." : "Scan"
                        enabled: !root.isScanning
                        onClicked: {
                            networkManager.requestScan()
                            root.isScanning = true
                        }
                        Layout.preferredWidth: 100
                        Material.background: "#1e1e2e"
                    }
                }

                // Network list
                ListView {
                    width: parent.width
                    height: 300
                    clip: true
                    model: root.availableNetworks
                    spacing: 8
                    delegate: NetworkDelegate {
                        width: parent.width
                        selected: ssid === root.selectedSsid
                        onClicked: {
                            root.selectedSsid = ssid
                            wifiPasswordField.focus = true
                        }
                    }
                }

                // WiFi password input
                Item {
                    width: parent.width
                    visible: root.selectedSsid !== ""
                    Column {
                        spacing: 8
                        Text {
                            text: "Password for " + root.selectedSsid
                            color: "#aaaaaa"
                            font.pixelSize: 14
                        }
                        TextField {
                            id: wifiPasswordField
                            width: parent.width
                            placeholderText: "Enter WiFi password"
                            echoMode: TextField.Password
                            onTextChanged: {
                                root.setupData.wifiPassword = text
                                root.setupData.wifiSsid = root.selectedSsid
                                root.canProceed = text.length >= 8 || networkManager.ethernetConnected
                                root.canProceedChanged()
                            }
                            background: Rectangle {
                                radius: 8
                                color: "#1e1e2e"
                                border.color: wifiPasswordField.focus ? "#00d4aa" : "#333333"
                                border.width: wifiPasswordField.focus ? 2 : 1
                            }
                        }
                    }
                }
            }

            // Use Ethernet only checkbox
            Row {
                spacing: 12
                CheckBox {
                    id: useEthernetOnly
                    checked: root.setupData.useEthernet
                    onCheckedChanged: {
                        root.setupData.useEthernet = checked
                        root.canProceed = checked || (root.selectedSsid !== "" && root.setupData.wifiPassword.length >= 8)
                        root.canProceedChanged()
                    }
                    indicator: Rectangle {
                        width: 24
                        height: 24
                        radius: 6
                        color: checked ? "#00d4aa" : "#1e1e2e"
                        border.color: checked ? "#00d4aa" : "#333333"
                        border.width: 2
                    }
                }
                Text {
                    text: "Use Ethernet only (skip WiFi)"
                    color: "#ffffff"
                    font.pixelSize: 15
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        // Connection test
        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 200
            height: 44
            text: "Test Connection"
            enabled: root.canProceed && !networkManager.connecting
            onClicked: {
                if (root.setupData.useEthernet) {
                    // Ethernet is already connected
                } else if (root.selectedSsid !== "") {
                    networkManager.connectToWifi(root.selectedSsid, root.setupData.wifiPassword)
                }
            }
            Material.background: "#00d4aa"
            Material.foreground: "#000000"
        }
    }
}

// Network delegate component
Component {
    id: networkDelegateComponent
    Item {
        id: delegateRoot
        width: parent.width
        height: 56
        property string ssid: model.ssid
        property int strength: model.strength
        property bool secured: model.secured
        property bool selected: false
        property bool connected: model.connected

        signal clicked()

        Rectangle {
            anchors.fill: parent
            radius: 10
            color: selected ? "#002211" : (delegateRoot.MouseArea.pressed ? "#2a2a3a" : "#1e1e2e")
            border.color: selected ? "#00d4aa" : (connected ? "#00d4aa" : "#333333")
            border.width: selected || connected ? 2 : 1

            Row {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Image {
                    source: "qrc:/icons/wifi-warning.svg"
                    width: 24
                    height: 24
                    color: connected ? "#00d4aa" : "#888888"
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2
                    Text {
                        text: ssid
                        color: "#ffffff"
                        font.pixelSize: 15
                        font.bold: selected || connected
                    }
                    Row {
                        spacing: 4
                        Repeater {
                            model: 4
                            delegate: Rectangle {
                                width: 4
                                height: (index + 1) * 4
                                radius: 2
                                color: index < Math.round(strength / 25) ? (connected ? "#00d4aa" : "#888888") : "#333333"
                            }
                        }
                        Text {
                            text: secured ? "🔒" : "🔓"
                            font.pixelSize: 12
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: connected ? "Connected" : ""
                    color: "#00d4aa"
                    font.pixelSize: 13
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                id: delegateRoot.MouseArea
                anchors.fill: parent
                onClicked: delegateRoot.clicked()
            }
        }
    }
}