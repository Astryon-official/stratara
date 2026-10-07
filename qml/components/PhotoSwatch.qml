import QtQuick
import QtQuick.Controls

Item {
    id: root
    property bool selected: false
    property string imagePath: ""
    property string placeholderText: "Choose photo"

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

        Image {
            id: image
            anchors.fill: parent
            source: root.imagePath && root.imagePath.length > 0 ? "file://" + root.imagePath : ""
            fillMode: Image.PreserveAspectCrop
            visible: root.imagePath && root.imagePath.length > 0
        }

        Text {
            visible: !root.imagePath || root.imagePath.length === 0
            text: placeholderText
            font.pixelSize: 12
            color: "#888888"
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            width: parent.width - 20
        }

        Image {
            visible: selected
            source: "qrc:/icons/check.svg"
            width: 24
            height: 24
            color: "#ffffff"
            anchors.centerIn: parent
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked()
        }
    }
}