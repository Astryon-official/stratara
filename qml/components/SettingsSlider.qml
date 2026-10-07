import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property string label
    property real from: 0
    property real to: 100
    property real value: 50
    property real stepSize: 1
    property bool enabled: true

    signal valueChanged(real value)

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

        Item {
            Layout.preferredWidth: 200
            Layout.alignment: Qt.AlignRight

            RowLayout {
                anchors.fill: parent
                spacing: 12

                Slider {
                    id: slider
                    from: root.from
                    to: root.to
                    value: root.value
                    stepSize: root.stepSize
                    enabled: root.enabled
                    Layout.fillWidth: true

                    background: Rectangle {
                        height: 4
                        radius: 2
                        color: "#ffffff22"
                    }

                    handle: Rectangle {
                        width: 20
                        height: 20
                        radius: 10
                        color: "#00d4ff"
                        border.color: "#ffffff"
                        border.width: 2
                    }

                    progress: Rectangle {
                        height: 4
                        radius: 2
                        color: "#00d4ff"
                    }

                    onValueChanged: {
                        root.value = value
                        root.valueChanged(value)
                    }
                }

                Label {
                    text: Qt.formatNumber(slider.value, 'f', slider.stepSize < 1 ? 1 : 0)
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "#ffffff"
                    Layout.preferredWidth: 60
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }
}