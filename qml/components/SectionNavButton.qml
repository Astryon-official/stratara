import QtQuick
import QtQuick.Controls

Item {
    id: root
    property string title
    property bool active: false
    property var focusRequester: null

    signal clicked()

    width: parent.width
    height: 56

    Rectangle {
        id: bg
        anchors.fill: parent
        color: active ? "#ffffff1a" : "transparent"
        border.color: "#ffffff11"
        border.width: 1

        Text {
            text: root.title
            font.pixelSize: 19
            font.weight: Font.Medium
            color: active ? "#ffffff" : "#aaaaaa"
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 20
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked()
        }
    }

    focusPolicy: Qt.StrongFocus
    FocusRequester { id: focusRequester }
    onFocusChanged: {
        if (focus && focusRequester) focusRequester.requestFocus()
    }
}