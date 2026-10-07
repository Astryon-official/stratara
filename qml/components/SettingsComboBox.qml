import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property string label
    property var model: []
    property int currentIndex: 0
    property string currentText: ""
    property bool enabled: true

    signal currentIndexChanged(int index)
    signal currentTextChanged(string text)

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

        ComboBox {
            id: comboBox
            model: root.model
            currentIndex: root.currentIndex
            currentText: root.currentText
            enabled: root.enabled
            Layout.preferredWidth: 200
            Layout.alignment: Qt.AlignRight

            background: Rectangle {
                radius: 8
                color: "#2a2a3a"
                border.color: "#ffffff22"
                border.width: 1
            }

            indicator: Image {
                source: "qrc:/icons/chevron-down.svg"
                width: 16
                height: 16
                color: "#ffffff"
            }

            contentItem: Text {
                text: comboBox.currentText
                font.pixelSize: 16
                color: "#ffffff"
                leftPadding: 12
                rightPadding: 32
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            popup: Popup {
                y: comboBox.height - 1
                width: comboBox.width
                implicitHeight: contentItem.implicitHeight
                padding: 0

                contentItem: ListView {
                    clip: true
                    model: comboBox.model
                    currentIndex: comboBox.currentIndex

                    delegate: ItemDelegate {
                        text: modelData
                        width: parent.width
                        height: 44
                        font.pixelSize: 16
                        highlighted: ListView.isCurrentItem
                        onClicked: {
                            comboBox.currentIndex = index
                            comboBox.popup.close()
                        }
                    }

                    ScrollBar.vertical: ScrollBar { }
                }

                background: Rectangle {
                    radius: 8
                    color: "#1a1a2e"
                    border.color: "#ffffff22"
                    border.width: 1
                }
            }

            onCurrentIndexChanged: {
                root.currentIndex = currentIndex
                root.currentIndexChanged(currentIndex)
            }
            onCurrentTextChanged: {
                root.currentText = currentText
                root.currentTextChanged(currentText)
            }
        }
    }
}