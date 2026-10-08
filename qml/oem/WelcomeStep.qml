import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property bool canProceed: true

    signal canProceedChanged()

    Column {
        anchors.centerIn: parent
        spacing: 32
        width: parent.width

        // Stratara logo
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/icons/stratara.svg"
            width: 180
            height: 180
            fillMode: Image.PreserveAspectFit
        }

        // Title
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Welcome to Stratara"
            color: "#ffffff"
            font.pixelSize: 42
            font.bold: true
            font.weight: Font.ExtraBold
        }

        // Subtitle
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Your Linux living-room experience begins here"
            color: "#aaaaaa"
            font.pixelSize: 20
            font.weight: Font.Light
        }

        // Feature highlights
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16
            width: Math.min(parent.width, 800)

            Repeater {
                model: [
                    { icon: "🎮", title: "Controller-First", desc: "Designed for TV remotes and game controllers" },
                    { icon: "🖥️", title: "Native Linux", desc: "Real Linux apps, Wayland, systemd, KWin" },
                    { icon: "🔧", title: "Extensible", desc: "Xodus system layer, OTA updates, modular" },
                    { icon: "🎨", title: "Beautiful", desc: "Glass morphism, animations, Frame Art mode" }
                ]
                delegate: Row {
                    spacing: 20
                    width: parent.width

                    Text {
                        text: modelData.icon
                        font.pixelSize: 32
                    }

                    Column {
                        spacing: 4
                        Text {
                            text: modelData.title
                            color: "#ffffff"
                            font.pixelSize: 18
                            font.bold: true
                        }
                        Text {
                            text: modelData.desc
                            color: "#888888"
                            font.pixelSize: 14
                        }
                    }
                }
            }
        }

        // Continue hint
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Click Next to begin setup"
            color: "#00d4aa"
            font.pixelSize: 14
            opacity: 0.8
        }
    }
}