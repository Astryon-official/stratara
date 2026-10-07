import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property AppModel appModel
    property Settings settings
    property int columns: 4

    anchors.fill: parent

    // Dock (favorites) - horizontal row at bottom
    Dock {
        id: dock
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 140
        margins: Qt.size(56, 0, 56, 40)
        appModel: appModel
        settings: settings
        glassBlur: settings.glassBlur
        glassBackdrop: root.glassBackdrop
    }

    // App grid - fills remaining space
    AppGrid {
        id: grid
        anchors.top: parent.top
        anchors.bottom: dock.top
        anchors.left: parent.left
        anchors.right: parent.right
        margins: Qt.size(56, 20, 56, 20)
        appModel: appModel
        settings: settings
        columns: settings.columns
        glassBlur: settings.glassBlur
        glassBackdrop: root.glassBackdrop
    }

    // Glass backdrop reference
    property Item glassBackdrop: null
}