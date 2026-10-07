import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var appInfo
    property bool isArtworkEnabled: false

    signal toggled(bool enabled)

    width: parent.width
    height: 56

    RowLayout {
        anchors.fill: parent
        spacing: 16

        // App icon
        Image {
            source: appInfo.iconName ? "image://appicons/" + appInfo.iconName : "qrc:/icons/app-placeholder.svg"
            width: 40
            height: 40
            fillMode: Image.PreserveAspectFit
            smooth: true
            Layout.alignment: Qt.AlignVCenter
        }

        // App name
        Label {
            text: appInfo.name
            font.pixelSize: 16
            color: "#ffffff"
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        // Artwork toggle
        Switch {
            checked: isArtworkEnabled
            onCheckedChanged: root.toggled(checked)
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }
    }
}