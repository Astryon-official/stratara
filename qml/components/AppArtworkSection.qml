import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property FavoritesStore favoritesStore
    property AppModel appModel

    anchors.fill: parent
    height: childrenRect.height

    Column {
        id: column
        spacing: 22

        SectionLabel { text: "App artwork on hover" }

        Row {
            spacing: 16
            ToggleChip { text: "On"; checked: settings.useAppArtwork; onClicked: settings.useAppArtwork = true }
            ToggleChip { text: "Off"; checked: !settings.useAppArtwork; onClicked: settings.useAppArtwork = false }
        }
        Text {
            text: "While a favorite is focused, its TV artwork takes over the wallpaper."
            color: "#aaaaaa"
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            width: parent.width * 0.85
        }

        Loader {
            visible: settings.useAppArtwork
            sourceComponent: artworkAppsList
        }
    }

    Component {
        id: artworkAppsList
        Column {
            spacing: 12
            Repeater {
                model: appModel
                delegate: AppArtworkToggle {
                    width: parent.width
                    appInfo: model
                    isArtworkEnabled: settings.artworkApps.indexOf(model.packageName) >= 0
                    onToggled: {
                        var artworkApps = settings.artworkApps
                        if (enabled) {
                            artworkApps.push(model.packageName)
                        } else {
                            artworkApps = artworkApps.filter(function(pkg) { return pkg !== model.packageName })
                        }
                        settings.artworkApps = artworkApps
                    }
                }
            }
        }
    }
}