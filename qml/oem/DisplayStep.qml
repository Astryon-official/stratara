import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var setupData
    property bool canProceed: true
    property bool calibrating: false
    property int calibrationStep: 0
    property var calibrationPoints: []
    property var testPattern: 0

    signal canProceedChanged()

    Column {
        anchors.centerIn: parent
        spacing: 24
        width: parent.width

        // Title
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Display Setup"
            color: "#ffffff"
            font.pixelSize: 36
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Configure your display for the best living-room experience"
            color: "#888888"
            font.pixelSize: 16
        }

        // Resolution selector
        Column {
            spacing: 16
            width: Math.min(parent.width, 600)
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                text: "Display Resolution"
                color: "#aaaaaa"
                font.pixelSize: 14
            }

            Row {
                spacing: 12
                Repeater {
                    model: ["1920x1080", "1280x720", "3840x2160", "Auto"]
                    delegate: Button {
                        text: modelData
                        checkable: true
                        checked: root.setupData.displayMode === modelData
                        onClicked: {
                            root.setupData.displayMode = modelData
                        }
                        Layout.preferredHeight: 44
                        Layout.fillWidth: true
                        Material.background: checked ? "#00d4aa" : "#1e1e2e"
                        Material.foreground: checked ? "#000000" : "#ffffff"
                    }
                }
            }

            // Refresh rate
            Text {
                text: "Refresh Rate"
                color: "#aaaaaa"
                font.pixelSize: 14
            }

            Row {
                spacing: 12
                Repeater {
                    model: ["60Hz", "120Hz", "Auto"]
                    delegate: Button {
                        text: modelData
                        checkable: true
                        Layout.preferredHeight: 44
                        Layout.fillWidth: true
                        Material.background: "#1e1e2e"
                    }
                }
            }
        }

        // Overscan calibration
        Rectangle {
            width: Math.min(parent.width, 700)
            height: 320
            anchors.horizontalCenter: parent.horizontalCenter
            radius: 16
            color: "#000000"
            border.color: "#333333"
            border.width: 2

            // Overscan test pattern
            Item {
                anchors.fill: parent
                anchors.margins: 20

                // Corner markers
                Repeater {
                    model: 4
                    delegate: Rectangle {
                        id: corner
                        width: 3
                        height: 3
                        color: "#00d4aa"
                        x: index % 2 === 0 ? 0 : parent.width - 3
                        y: index < 2 ? 0 : parent.height - 3
                    }
                }

                // Edge markers
                Repeater {
                    model: 4
                    delegate: Rectangle {
                        id: edge
                        color: "#00d4aa"
                        x: index === 0 ? 0 : (index === 1 ? parent.width - 3 : parent.width / 2 - 1.5)
                        y: index === 2 ? 0 : (index === 3 ? parent.height - 3 : parent.height / 2 - 1.5)
                        width: index < 2 ? 3 : parent.width
                        height: index < 2 ? parent.height : 3
                        opacity: 0.3
                    }
                }

                // Center crosshair
                Item {
                    anchors.centerIn: parent
                    width: 60
                    height: 60

                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 2
                        height: parent.height
                        color: "#00d4aa"
                        opacity: 0.5
                    }
                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        height: 2
                        color: "#00d4aa"
                        opacity: 0.5
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "✓"
                        color: "#00d4aa"
                        font.pixelSize: 32
                        font.bold: true
                    }
                }

                // Instructions
                Text {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.margins: 16
                    text: "Adjust your TV's overscan/settings so all corners and edges are visible"
                    color: "#888888"
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // Calibration controls
        Column {
            spacing: 16
            anchors.horizontalCenter: parent.horizontalCenter

            Row {
                spacing: 16

                Button {
                    text: "Test Pattern"
                    onClicked: {
                        root.testPattern = (root.testPattern + 1) % 3
                    }
                    Layout.preferredWidth: 160
                    Layout.preferredHeight: 44
                    Material.background: "#1e1e2e"
                }

                Button {
                    text: "Color Bars"
                    onClicked: {
                        root.testPattern = 3
                    }
                    Layout.preferredWidth: 160
                    Layout.preferredHeight: 44
                    Material.background: "#1e1e2e"
                }
            }

            Row {
                spacing: 16

                CheckBox {
                    id: hdrCheck
                    checked: false
                    text: "HDR Capable"
                    textColor: "#ffffff"
                    indicator: Rectangle {
                        width: 24
                        height: 24
                        radius: 6
                        color: checked ? "#00d4aa" : "#1e1e2e"
                        border.color: checked ? "#00d4aa" : "#333333"
                        border.width: 2
                    }
                }

                CheckBox {
                    id: dolbyCheck
                    checked: false
                    text: "Dolby Vision"
                    textColor: "#ffffff"
                    indicator: Rectangle {
                        width: 24
                        height: 24
                        radius: 6
                        color: checked ? "#00d4aa" : "#1e1e2e"
                        border.color: checked ? "#00d4aa" : "#333333"
                        border.width: 2
                    }
                }
            }
        }

        // Completion status
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Display looks good? Click Next to complete setup."
            color: "#00d4aa"
            font.pixelSize: 14
            font.bold: true
        }
    }

    // Test pattern overlay
    Loader {
        anchors.fill: parent
        visible: root.testPattern > 0
        sourceComponent: testPatternComponent
    }

    Component {
        id: testPatternComponent
        Item {
            anchors.fill: parent

            // White
            Rectangle {
                visible: root.testPattern === 1
                anchors.fill: parent
                color: "#ffffff"
            }

            // Black
            Rectangle {
                visible: root.testPattern === 2
                anchors.fill: parent
                color: "#000000"
            }

            // Red
            Rectangle {
                visible: root.testPattern === 3
                anchors.fill: parent
                color: "#ff0000"
            }

            // Close button
            Button {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 40
                text: "✕"
                onClicked: {
                    root.testPattern = 0
                }
                width: 48
                height: 48
                border.radius: 24
                Material.background: "#1e1e2e"
            }
        }
    }

    Component.onCompleted: {
        root.canProceed = true
        root.canProceedChanged()
    }
}