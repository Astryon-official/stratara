import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property string label
    property bool checked: false
    property bool enabled: true

    signal checkedChanged(bool checked)

    width: parent ? parent.width : 400
    height: 56

    RowLayout {
        anchors.fill: parent
        anchors.margins: Qt.size(16, 0, 16, 0)

        Label {
            text: label
            font.pixelSize: 16
            color: enabled ? "#ffffff" : "#888888"
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
        }

        Switch {
            checked: root.checked
            enabled: root.enabled
            onCheckedChanged: root.checkedChanged(checked)
            Layout.alignment: Qt.AlignRight
        }
    }
}