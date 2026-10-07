import QtQuick
import QtQuick.Controls

Item {
    id: root
    property int index: 0
    property bool selected: false
    property variant preset: {}

    signal clicked()

    width: 80
    height: 80

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 12
        color: selected ? "#00d4ff" : "#2a2a2c"
        border.color: selected ? "#00d4ff" : "#ffffff22"
        border.width: selected ? 3 : 1

        gradient: Gradient {
            GradientStop { position: 0.0; color: root.preset.color1 || "#0a0a0c" }
            GradientStop { position: 0.5; color: root.preset.color2 || "#1a1a2e" }
            GradientStop { position: 1.0; color: root.preset.color3 || "#16213e" }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked()
        }
    }

    Image {
        visible: selected
        source: "qrc:/icons/check.svg"
        width: 24
        height: 24
        color: "#ffffff"
        anchors.centerIn: parent
    }
}