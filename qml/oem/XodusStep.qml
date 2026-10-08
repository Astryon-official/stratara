import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var setupData
    property bool canProceed: true
    property bool enrolling: false
    property string enrollStatus: ""
    property bool enrolled: false

    signal canProceedChanged()

    // Xodus manager integration
    Connections {
        target: xodusManager
        onRegistrationResult: {
            root.enrolling = false
            if (success) {
                root.enrolled = true
                root.enrollStatus = "Successfully enrolled with Xodus!"
            } else {
                root.enrollStatus = "Enrollment failed: " + error
            }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 24
        width: parent.width

        // Title
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Xodus Enrollment"
            color: "#ffffff"
            font.pixelSize: 36
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Register this device with the Xodus system layer for remote management"
            color: "#888888"
            font.pixelSize: 16
        }

        // What is Xodus
        Rectangle {
            width: Math.min(parent.width, 700)
            height: 140
            anchors.horizontalCenter: parent.horizontalCenter
            radius: 16
            color: "#1e1e2e"
            border.color: "#333333"
            border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 12
                anchors.margins: 24

                Row {
                    spacing: 12
                    Image {
                        source: "qrc:/icons/stratara.svg"
                        width: 32
                        height: 32
                        color: "#00d4aa"
                    }
                    Text {
                        text: "What is Xodus?"
                        color: "#ffffff"
                        font.pixelSize: 18
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Text {
                    text: "Xodus is the Stratara system layer that provides:\n• Remote device management\n• System health monitoring\n• OTA update coordination\n• Centralized configuration"
                    color: "#aaaaaa"
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
            }
        }

        // Enrollment form
        Column {
            spacing: 20
            width: Math.min(parent.width, 500)
            anchors.horizontalCenter: parent.horizontalCenter

            // Enable enrollment checkbox
            Row {
                spacing: 12
                CheckBox {
                    id: enrollCheck
                    checked: setupData.enrollXodus
                    onCheckedChanged: {
                        setupData.enrollXodus = checked
                        root.canProceed = !checked || (xodusNameField.text.length > 0)
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
                Column {
                    spacing: 2
                    Text {
                        text: "Enroll with Xodus"
                        color: "#ffffff"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    Text {
                        text: "Recommended for managed deployments"
                        color: "#888888"
                        font.pixelSize: 12
                    }
                }
            }

            // Device name
            Column {
                spacing: 8
                visible: enrollCheck.checked
                Text {
                    text: "Device Name"
                    color: "#aaaaaa"
                    font.pixelSize: 14
                }
                TextField {
                    id: xodusNameField
                    width: parent.width
                    placeholderText: "e.g. Living Room Stratara"
                    text: setupData.xodusName
                    onTextChanged: {
                        setupData.xodusName = text
                        root.canProceed = text.length > 0
                        root.canProceedChanged()
                    }
                    background: Rectangle {
                        radius: 8
                        color: "#1e1e2e"
                        border.color: xodusNameField.focus ? "#00d4aa" : "#333333"
                        border.width: xodusNameField.focus ? 2 : 1
                    }
                }
            }

            // Device type
            Column {
                spacing: 8
                visible: enrollCheck.checked
                Text {
                    text: "Device Type"
                    color: "#aaaaaa"
                    font.pixelSize: 14
                }
                ComboBox {
                    id: typeCombo
                    width: parent.width
                    model: ["living-room-shell", "kiosk", "development", "testing"]
                    currentIndex: model.indexOf(setupData.xodusType)
                    onCurrentIndexChanged: {
                        setupData.xodusType = model[currentIndex]
                    }
                    background: Rectangle {
                        radius: 8
                        color: "#1e1e2e"
                        border.color: "#333333"
                        border.width: 1
                    }
                    delegate: ItemDelegate {
                        width: parent.width
                        text: modelData
                        highlighted: true
                        contentItem: Text {
                            text: modelData
                            color: "#ffffff"
                            font.pixelSize: 14
                            padding: 12
                        }
                    }
                }
            }

            // Description (optional)
            Column {
                spacing: 8
                visible: enrollCheck.checked
                Text {
                    text: "Description (optional)"
                    color: "#aaaaaa"
                    font.pixelSize: 14
                }
                TextField {
                    width: parent.width
                    placeholderText: "Brief description of this device"
                    text: setupData.xodusDescription || ""
                    onTextChanged: {
                        setupData.xodusDescription = text
                    }
                    background: Rectangle {
                        radius: 8
                        color: "#1e1e2e"
                        border.color: "#333333"
                        border.width: 1
                    }
                }
            }
        }

        // Enroll button / status
        Column {
            spacing: 16
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                id: enrollButton
                text: root.enrolling ? "Enrolling..." : (root.enrolled ? "Enrolled ✓" : "Enroll with Xodus")
                enabled: root.setupData.enrollXodus && !root.enrolling && !root.enrolled && xodusNameField.text.length > 0
                visible: !root.enrolled || root.enrolling
                onClicked: {
                    root.enrolling = true
                    root.enrollStatus = "Connecting to Xodus..."
                    xodusManager.registerService(
                        root.setupData.xodusName,
                        {
                            "type": root.setupData.xodusType,
                            "version": "0.1.0",
                            "description": root.setupData.xodusDescription || ""
                        }
                    )
                }
                Layout.preferredWidth: 280
                Layout.preferredHeight: 48
                font.pixelSize: 16
                Material.background: root.enrolled ? "#00d4aa" : "#00d4aa"
                Material.foreground: "#000000"
            }

            // Status message
            Text {
                text: root.enrollStatus
                color: root.enrollStatus.contains("failed") ? "#ff6b6b" : "#00d4aa"
                font.pixelSize: 14
                font.bold: true
                visible: root.enrollStatus !== ""
            }

            // Skip option
            Text {
                text: root.setupData.enrollXodus ? "" : "Xodus enrollment skipped. You can enroll later from Settings."
                color: "#666666"
                font.pixelSize: 13
                visible: !root.setupData.enrollXodus
            }
        }
    }

    Component.onCompleted: {
        root.canProceed = true
        root.canProceedChanged()
    }
}