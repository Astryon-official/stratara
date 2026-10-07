import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: root
    property string label
    property string filePath: ""
    property bool enabled: true

    signal filePathChanged(string path)

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

        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignRight

            TextField {
                id: pathField
                text: root.filePath
                placeholderText: "No file selected"
                readOnly: true
                enabled: root.enabled
                Layout.fillWidth: true
                Layout.minimumWidth: 200
                Layout.maximumWidth: 400

                background: Rectangle {
                    radius: 8
                    color: "#2a2a3a"
                    border.color: "#ffffff22"
                    border.width: 1
                }

                font.pixelSize: 14
                color: "#ffffff"
                placeholderTextColor: "#888888"
                leftPadding: 12
                rightPadding: 12
            }

            Button {
                id: browseBtn
                text: "Browse"
                enabled: root.enabled
                Layout.preferredWidth: 100

                background: Rectangle {
                    radius: 8
                    color: "#00d4ff"
                }

                contentItem: Text {
                    text: "Browse"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "#000000"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: fileDialog.open()
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select Image"
        folder: Qt.resolvedUrl("file://" + Qt.standardPaths.standardLocations(Qt.PicturesLocation)[0])
        nameFilters: ["Images (*.png *.jpg *.jpeg *.webp *.bmp)", "All Files (*)"]
        selectExisting: true
        selectMultiple: false

        onAccepted: {
            root.filePath = fileDialog.fileUrl.toString().replace("file://", "")
            root.filePathChanged(root.filePath)
            pathField.text = root.filePath
        }
    }
}