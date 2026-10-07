import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

Item {
    id: root
    property bool glassBlur: false
    property Item glassBackdrop: null
    property alias clock: clock
    property alias nowPlayingChip: nowPlayingChip

    height: 80
    width: parent.width

    // Frosted glass background
    Rectangle {
        id: glassBg
        anchors.fill: parent
        visible: glassBlur && glassBackdrop
        layer.enabled: true
        layer.effect: FastBlur {
            radius: 24
            samples: 8
        }
        layer.samplerName: "backdrop"
        ShaderEffectSource {
            id: backdrop
            sourceItem: glassBackdrop
            live: true
            format: ShaderEffectSource.RGBA8888
        }
    }

    // Opaque fallback
    Rectangle {
        id: opaqueBg
        anchors.fill: parent
        visible: !glassBlur
        color: "#1a1a1aff"
        opacity: 0.85
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Qt.size(56, 20, 56, 12)

        // Left group: clock + weather
        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 12

            // Clock
            Rectangle {
                id: clockContainer
                height: 56
                radius: 28
                color: glassBlur ? "transparent" : "#2a2a2c"
                border.color: glassBlur ? "#ffffff33" : "transparent"
                border.width: glassBlur ? 1 : 0
                layer.enabled: glassBlur
                layer.effect: FastBlur { radius: 16; samples: 4 }

                Label {
                    id: clock
                    anchors.centerIn: parent
                    text: Qt.formatTime(new Date(), "hh:mm")
                    font.pixelSize: 28
                    font.weight: Font.Medium
                    color: "#ffffff"
                    padding: Qt.size(20, 0)
                }

                Timer {
                    interval: 1000
                    running: true
                    repeat: true
                    onTriggered: clock.text = Qt.formatTime(new Date(), "hh:mm")
                }
            }

            // Weather (placeholder)
            Rectangle {
                id: weatherContainer
                height: 56
                radius: 28
                visible: false // TODO: connect to weather service
                color: glassBlur ? "transparent" : "#2a2a2c"
                border.color: glassBlur ? "#ffffff33" : "transparent"
                border.width: glassBlur ? 1 : 0

                Row {
                    anchors.centerIn: parent
                    spacing: 6
                    Label {
                        text: "☀️"
                        font.pixelSize: 20
                    }
                    Label {
                        text: "22°C"
                        font.pixelSize: 22
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                }
                padding: Qt.size(16, 0)
            }
        }

        // Center: Now playing
        Item {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                id: nowPlayingContainer
                height: 56
                radius: 28
                visible: false // TODO: connect to MPRIS
                color: glassBlur ? "transparent" : "#2a2a2c"
                border.color: glassBlur ? "#ffffff33" : "transparent"
                border.width: glassBlur ? 1 : 0

                Row {
                    anchors.centerIn: parent
                    spacing: 10
                    padding: Qt.size(14, 0)

                    Rectangle {
                        width: 30
                        height: 30
                        radius: 8
                        color: "#444"
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        Label {
                            text: "Track Title"
                            font.pixelSize: 16
                            font.weight: Font.Medium
                            color: "#ffffff"
                        }
                        Label {
                            text: "Artist Name"
                            font.pixelSize: 13
                            color: "#aaaaaa"
                        }
                    }
                }
            }
        }

        // Right group: status + actions
        RowLayout {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            spacing: 4

            // Network status
            Rectangle {
                id: networkContainer
                height: 56
                width: 56
                radius: 28
                visible: false // TODO: connect to NetworkManager
                color: glassBlur ? "transparent" : "#2a2a2c"
                border.color: glassBlur ? "#ffaa00" : "transparent"
                border.width: glassBlur ? 2 : 0

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/icons/wifi-warning.svg"
                    width: 22
                    height: 22
                    color: "#ffaa00"
                }
            }

            // Frame Art button
            Button {
                id: frameButton
                width: 56
                height: 56
                radius: 28
                background: Rectangle {
                    anchors.fill: parent
                    radius: 28
                    color: glassBlur ? "transparent" : "#2a2a2c"
                    border.color: glassBlur ? "#ffffff33" : "transparent"
                    border.width: glassBlur ? 1 : 0
                }
                contentItem: Image {
                    source: "qrc:/icons/frame.svg"
                    width: 22
                    height: 22
                    color: "#ffffff"
                }
                onClicked: {
                    // TODO: Enter Frame Art mode
                }
            }

            // Settings button
            Button {
                id: settingsButton
                width: 56
                height: 56
                radius: 28
                background: Rectangle {
                    anchors.fill: parent
                    radius: 28
                    color: glassBlur ? "transparent" : "#2a2a2c"
                    border.color: glassBlur ? "#ffffff33" : "transparent"
                    border.width: glassBlur ? 1 : 0
                }
                contentItem: Image {
                    source: "qrc:/icons/settings.svg"
                    width: 22
                    height: 22
                    color: "#ffffff"
                }
                onClicked: {
                    // TODO: Open settings
                }
            }
        }
    }
}