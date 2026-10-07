import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    default property alias content: column.children

    Column {
        id: column
        anchors.fill: parent
        spacing: 16
    }
}