import QtQuick
import QtQuick.Controls

Item {
    id: root
    property string text
    property bool checked: false
    property bool active: false

    signal clicked()

    width: implicitWidth
    height: 40

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 20
        color: checked ? "#00d4ff" : (active ? "#ffffff1a" : "transparent")
        border.color: active ? "#ffffff44" : "#ffffff22"
        border.width: active ? 2 : 1

        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked()
        }
    }

    Text {
        text: root.text
        font.pixelSize: 14
        font.weight: Font.Medium
        color: checked ? "#000000" : (active ? "#ffffff" : "#aaaaaa")
        anchors.centerIn: parent
        padding: Qt.size(16, 8)
    }
}