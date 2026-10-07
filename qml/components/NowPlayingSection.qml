import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings

    anchors.fill: parent
    height: column.height

    Column {
        id: column
        spacing: 16

        SectionLabel { text: "Now playing" }

        Row {
            spacing: 16
            ToggleChip { text: "On"; checked: settings.nowPlaying; onClicked: settings.nowPlaying = true }
            ToggleChip { text: "Off"; checked: !settings.nowPlaying; onClicked: settings.nowPlaying = false }
        }
        Text {
            text: "Shows a compact chip in the top bar with whatever's currently playing."
            color: "#aaaaaa"
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            width: parent.width * 0.85
        }

        Loader {
            visible: settings.nowPlaying
            sourceComponent: notificationAccessContent
        }
    }

    Component {
        id: notificationAccessContent
        Column {
            spacing: 12
            Row {
                spacing: 12
                Image {
                    source: notificationAccessGranted ? "qrc:/icons/check.svg" : "qrc:/icons/alert.svg"
                    width: 20
                    height: 20
                    color: notificationAccessGranted ? "#00ff00" : "#ffaa00"
                }
                Label {
                    text: notificationAccessGranted ? "Notification access: Granted" : "Needs notification access to read what's playing"
                    font.pixelSize: 13
                    color: notificationAccessGranted ? "#00ff00" : "#ffaa00"
                    verticalAlignment: Text.AlignVCenter
                }
            }
            ToggleChip {
                visible: !notificationAccessGranted
                text: "Grant notification access"
                checked: false
                onClicked: {
                    Qt.openUrlExternally("settings://notification-access")
                }
            }
        }
    }

    property bool notificationAccessGranted: false
}