import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property AppModel appModel
    property Settings settings
    property bool glassBlur: false
    property Item glassBackdrop: null

    height: 140

    // Get favorites from settings (for now, just show first 5 apps)
    property var favorites: []

    Component.onCompleted: {
        updateFavorites()
    }

    function updateFavorites() {
        var favs = []
        // For now, just use first 5 apps as "favorites"
        for (var i = 0; i < Math.min(5, appModel.count); i++) {
            favs.push(appModel.getApp(i))
        }
        favorites = favs
    }

    // Frosted glass background
    Rectangle {
        id: dockBg
        anchors.fill: parent
        radius: 28
        visible: glassBlur && glassBackdrop
        layer.enabled: true
        layer.effect: FastBlur {
            radius: 32
            samples: 16
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
        id: opaqueDockBg
        anchors.fill: parent
        radius: 28
        visible: !glassBlur
        color: "#1a1a1aff"
        opacity: 0.9
    }

    // Dock content
    Row {
        id: dockRow
        anchors.fill: parent
        anchors.margins: Qt.size(16, 12, 16, 12)
        spacing: 24

        Repeater {
            model: favorites
            delegate: AppCard {
                width: 112
                height: 112
                appInfo: modelData
                isDockItem: true
                glassBlur: root.glassBlur
                onLaunch: {
                    appModel.launchApp(appInfo.packageName)
                }
            }
        }

        // Add to dock button (when less than 5 items)
        Item {
            width: 112
            height: 112
            visible: favorites.length < 5

            Button {
                anchors.fill: parent
                background: Rectangle {
                    anchors.fill: parent
                    radius: 16
                    color: "transparent"
                    border.color: "#ffffff33"
                    border.width: 1
                    border.style: Qt.DashLine
                }
                contentItem: Image {
                    source: "qrc:/icons/add.svg"
                    width: 32
                    height: 32
                    color: "#ffffff88"
                }
                onClicked: {
                    // TODO: Show add to dock dialog
                }
            }
        }
    }
}