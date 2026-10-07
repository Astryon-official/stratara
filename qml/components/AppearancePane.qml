import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property FavoritesStore favoritesStore
    property AppModel appModel

    anchors.fill: parent
    focus: true

    Flickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height + 44
        clip: true
        ScrollBar.vertical: ScrollBar {
            width: 8
            policy: ScrollBar.AlwaysOn
            background: Rectangle { color: "transparent" }
            contentItem: Rectangle {
                radius: 4
                color: "#ffffff44"
            }
        }

        Column {
            id: column
            spacing: 22
            anchors.margins: Qt.size(40, 24, 40, 44)

            PaneTitle { text: "Appearance" }

            SectionLabel { text: "Theme" }
            Row {
                spacing: 16
                ToggleChip { text: "Dark"; checked: settings.theme === "Dark"; onClicked: settings.theme = "Dark" }
                ToggleChip { text: "Light"; checked: settings.theme === "Light"; onClicked: settings.theme = "Light" }
                ToggleChip { text: "Auto"; checked: settings.theme === "Auto"; onClicked: settings.theme = "Auto" }
            }
            Loader {
                visible: settings.theme === "Auto"
                sourceComponent: autoThemeHint
            }

            SectionLabel { text: "Tiles per row" }
            Row {
                spacing: 12
                Repeater {
                    model: 4
                    delegate: ToggleChip {
                        text: (index + 4).toString()
                        checked: settings.columns === (index + 4)
                        onClicked: settings.columns = index + 4
                    }
                }
            }

            SectionLabel { text: "Wallpaper" }
            Row {
                spacing: 10
                Repeater {
                    model: 4
                    delegate: PresetSwatch {
                        selected: !settings.useImageWallpaper && settings.wallpaperId === index
                        onClicked: {
                            settings.wallpaperId = index
                            settings.useImageWallpaper = false
                        }
                    }
                }
                PhotoSwatch {
                    selected: settings.useImageWallpaper
                    imagePath: settings.wallpaperImagePath
                    onClicked: {
                        if (!settings.wallpaperImagePath || settings.wallpaperImagePath.length === 0) {
                            fileDialog.open()
                        } else if (!settings.useImageWallpaper) {
                            settings.useImageWallpaper = true
                        } else {
                            fileDialog.open()
                        }
                    }
                }
            }

            FileDialog {
                id: fileDialog
                title: "Select Wallpaper"
                folder: Qt.standardPaths.standardLocations(Qt.PicturesLocation)[0]
                nameFilters: ["Images (*.png *.jpg *.jpeg *.webp *.bmp)", "All Files (*)"]
                selectExisting: true
                selectMultiple: false
                onAccepted: {
                    settings.wallpaperImagePath = fileDialog.fileUrl.toString().replace("file://", "")
                    settings.useImageWallpaper = true
                }
            }

            SectionLabel { text: "Frame Art as wallpaper" }
            Row {
                spacing: 16
                ToggleChip { text: "On"; checked: settings.useFrameArtWallpaper; onClicked: settings.useFrameArtWallpaper = true }
                ToggleChip { text: "Off"; checked: !settings.useFrameArtWallpaper; onClicked: settings.useFrameArtWallpaper = false }
            }
            Text {
                text: "Plays your Frame Art as the home wallpaper (a calm still frame), so opening Frame Art simply dissolves the launcher away and the painting comes alive. Set the source up under Frame Art."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            // App Artwork Section
            AppArtworkSection {
                settings: settings
                favoritesStore: favoritesStore
                appModel: appModel
            }

            SectionLabel { text: "Glass blur" }
            Row {
                spacing: 16
                ToggleChip { text: "On"; checked: settings.glassBlur; onClicked: settings.glassBlur = true }
                ToggleChip { text: "Off"; checked: !settings.glassBlur; onClicked: settings.glassBlur = false }
            }
            Text {
                text: "Frosts the wallpaper behind the top bar and dock. Turning it off (a flat tint instead) is the single biggest GPU saving on a slower TV."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            // Now Playing Section
            NowPlayingSection {
                settings: settings
            }

            SectionLabel { text: "Navigation sounds" }
            Row {
                spacing: 16
                ToggleChip { text: "On"; checked: settings.navSounds; onClicked: settings.navSounds = true }
                ToggleChip { text: "Off"; checked: !settings.navSounds; onClicked: settings.navSounds = false }
            }
            Text {
                text: "A soft tick on D-pad moves, a click when you open something, and a whoosh on the way back."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }
        }
    }

    Component {
        id: autoThemeHint
        Text {
            text: "Light from 7am to 7pm, dark otherwise."
            color: "#888888"
            font.pixelSize: 13
            width: parent.width
        }
    }
}