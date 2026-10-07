import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property AppModel appModel

    anchors.fill: parent
    focus: true

    Flickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height + 44
        clip: true
        ScrollBar.vertical: ScrollBar {
            width: 8
            policy: ScrollBar.AlwaysOn
            background: Rectangle { color: "transparent" }
            contentItem: Rectangle {
                radius: 4
                color: "#ffffff44"
            }
        }

        Column {
            id: column
            spacing: 22
            anchors.margins: Qt.size(40, 24, 40, 44)

            PaneTitle { text: "Home Setup" }
            Text {
                text: "Configure how Stratara integrates with the system as your home launcher."
                color: "#aaaaaa"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Accessibility" }
            Row {
                spacing: 16
                ToggleChip {
                    text: "Open Accessibility Settings"
                    checked: false
                    onClicked: {
                        Qt.openUrlExternally("settings://accessibility")
                    }
                }
            }
            Text {
                text: "Required for some launcher features like the 'Choose Home App' redirect."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Default Home App" }
            Row {
                spacing: 16
                ToggleChip {
                    text: "Set as Default Home"
                    checked: false
                    onClicked: {
                        Qt.openUrlExternally("settings://default-apps")
                    }
                }
            }
            Text {
                text: "Opens system settings to choose the default home launcher."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Android Settings" }
            Row {
                spacing: 16
                ToggleChip {
                    text: "Open Android Settings"
                    checked: false
                    onClicked: {
                        Qt.openUrlExternally("settings://")
                    }
                }
            }
        }
    }
}