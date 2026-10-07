import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Column {
    id: root
    property string title
    spacing: 12

    Label {
        text: title
        font.pixelSize: 18
        font.weight: Font.Bold
        color: "#ffffff"
        anchors.margins: Qt.size(16, 0, 16, 0)
    }

    Column {
        id: content
        spacing: 8
        anchors.margins: Qt.size(16, 0, 16, 16)

        default property alias children: content.data
    }
}