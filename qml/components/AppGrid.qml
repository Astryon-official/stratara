import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property AppModel appModel
    property Settings settings
    property int columns: 4
    property bool glassBlur: false
    property Item glassBackdrop: null

    anchors.fill: parent

    // Frosted glass background
    Rectangle {
        id: gridBg
        anchors.fill: parent
        radius: 16
        visible: glassBlur && glassBackdrop
        layer.enabled: true
        layer.effect: FastBlur {
            radius: 24
            samples: 12
        }
        layer.samplerName: "backdrop"
        ShaderEffectSource {
            sourceItem: glassBackdrop
            live: true
            format: ShaderEffectSource.RGBA8888
        }
    }

    // Opaque fallback
    Rectangle {
        id: opaqueGridBg
        anchors.fill: parent
        radius: 16
        visible: !glassBlur
        color: "#0a0a0c"
        opacity: 0.5
    }

    // App grid
    GridView {
        id: gridView
        anchors.fill: parent
        anchors.margins: Qt.size(16, 16, 16, 16)
        cellWidth: 240
        cellHeight: 200
        columnWidth: 240
        rowHeight: 200
        focus: true
        keyNavigationWraps: false
        clip: true

        // Calculate columns based on width
        property int calculatedColumns: Math.max(4, Math.min(7, Math.floor(width / 240)))
        flow: GridView.FlowLeftToRight

        model: appModel

        delegate: AppCard {
            width: gridView.cellWidth
            height: gridView.cellHeight
            appInfo: model
            isDockItem: false
            glassBlur: root.glassBlur
            onLaunch: {
                appModel.launchApp(appInfo.packageName)
            }
        }

        // Scrollbar
        ScrollBar.vertical: ScrollBar {
            width: 8
            policy: ScrollBar.AlwaysOn
            background: Rectangle { color: "transparent" }
            contentItem: Rectangle {
                radius: 4
                color: "#ffffff44"
            }
        }
    }
}