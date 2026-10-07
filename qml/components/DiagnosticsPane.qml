import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings

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

            PaneTitle { text: "Diagnostics" }
            Text {
                text: "System information and troubleshooting tools."
                color: "#aaaaaa"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "TV Content Scanner" }
            Row {
                spacing: 16
                ToggleChip {
                    text: "Scan for TV Content"
                    checked: false
                    onClicked: {
                        // TODO: Implement TV content scanning
                    }
                }
            }
            Text {
                text: "Scans for installed TV apps and media content."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "System Settings" }
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
            Text {
                text: "Opens the system settings app for advanced configuration."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Version Info" }
            Column {
                spacing: 8
                Row {
                    spacing: 16
                    Label { text: "Stratara Version"; font.pixelSize: 16; color: "#ffffff" }
                    Label { text: Qt.applicationVersion; font.pixelSize: 16; font.weight: Font.Medium; color: "#00d4ff" }
                }
                Row {
                    spacing: 16
                    Label { text: "Qt Version"; font.pixelSize: 16; color: "#ffffff" }
                    Label { text: Qt.version; font.pixelSize: 16; font.weight: Font.Medium; color: "#00d4ff" }
                }
            }
        }
    }
}