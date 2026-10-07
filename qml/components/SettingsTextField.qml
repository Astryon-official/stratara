import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property string label
    property string text: ""
    property string placeholderText: ""
    property bool enabled: true

    signal textChanged(string text)

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

        TextField {
            id: textField
            text: root.text
            placeholderText: root.placeholderText
            enabled: root.enabled
            Layout.preferredWidth: 300
            Layout.alignment: Qt.AlignRight

            background: Rectangle {
                radius: 8
                color: "#2a2a3a"
                border.color: "#ffffff22"
                border.width: 1
            }

            font.pixelSize: 16
            color: "#ffffff"
            placeholderTextColor: "#888888"
            leftPadding: 12
            rightPadding: 12

            onTextChanged: {
                root.text = text
                root.textChanged(text)
            }
        }
    }
}