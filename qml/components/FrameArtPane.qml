import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings

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

            PaneTitle { text: "Frame Art" }
            Text {
                text: "Turn the TV into a framed picture. Press the frame button in the top bar (or wait for the auto-start delay), then press any key to come back."
                color: "#aaaaaa"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Show" }
            Flow {
                width: parent.width
                spacing: 16
                ToggleChip { text: "Current wallpaper"; checked: settings.frameSource === "Wallpaper"; onClicked: settings.frameSource = "Wallpaper" }
                ToggleChip { text: "Folder"; checked: settings.frameSource === "Folder"; onClicked: settings.frameSource = "Folder" }
                ToggleChip { text: "Single photo"; checked: settings.frameSource === "Single"; onClicked: settings.frameSource = "Single" }
            }

            // Dynamic content based on frame source
            Loader {
                sourceComponent: frameSourceComponents[settings.frameSource]
            }

            SectionLabel { text: "Motion" }
            Row {
                spacing: 16
                ToggleChip { text: "On"; checked: settings.frameMotion; onClicked: settings.frameMotion = true }
                ToggleChip { text: "Off"; checked: !settings.frameMotion; onClicked: settings.frameMotion = false }
            }
            Text {
                text: "A very slow, smooth drift across the picture — a living painting. Off keeps it perfectly still."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Clock" }
            Row {
                spacing: 16
                ToggleChip { text: "On"; checked: settings.frameClock; onClicked: settings.frameClock = true }
                ToggleChip { text: "Off"; checked: !settings.frameClock; onClicked: settings.frameClock = false }
            }
            Text {
                text: "Shows an elegant time in the corner, floating above the art. Off is a pure picture."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            Loader {
                visible: settings.frameClock
                sourceComponent: clockOptions
            }

            SectionLabel { text: "Dim at night" }
            Row {
                spacing: 16
                ToggleChip { text: "On"; checked: settings.frameNightDim; onClicked: settings.frameNightDim = true }
                ToggleChip { text: "Off"; checked: !settings.frameNightDim; onClicked: settings.frameNightDim = false }
            }
            Text {
                text: "Gradually darkens the picture through the small hours so it isn't bright at 2am."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Auto-start" }
            Flow {
                width: parent.width
                spacing: 12
                Repeater {
                    model: [0, 60, 120, 300, 600, 1800]
                    delegate: ToggleChip {
                        text: modelData === 0 ? "Manual only" : (modelData < 60 ? modelData + " sec" : (modelData/60) + " min")
                        checked: settings.frameAutoStartSec === modelData
                        onClicked: settings.frameAutoStartSec = modelData
                    }
                }
            }
            Text {
                text: settings.frameAutoStartSec <= 0 ? "Frame Art starts only when you press the frame button." : "Frame Art also starts on its own after this long with no input."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }
        }
    }

    property var frameSourceComponents: {
        "Wallpaper": wallpaperContent,
        "Folder": folderContent,
        "Single": singleContent
    }

    Component {
        id: wallpaperContent
        Text {
            text: "Shows whatever wallpaper you've set, with no clock or app grid."
            color: "#aaaaaa"
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            width: parent.width * 0.85
        }
    }

    Component {
        id: folderContent
        Column {
            spacing: 22
            SectionLabel { text: "Folder" }
            ToggleChip {
                text: settings.frameFolderName || "Choose folder"
                checked: false
                onClicked: folderDialog.open()
            }

            SectionLabel { text: "Switch every" }
            Row {
                spacing: 12
                Repeater {
                    model: [10, 30, 60, 300, 900]
                    delegate: ToggleChip {
                        text: modelData === 60 ? "1 min" : (modelData < 60 ? modelData + " sec" : (modelData/60) + " min")
                        checked: settings.frameIntervalSec === modelData
                        onClicked: settings.frameIntervalSec = modelData
                    }
                }
            }
            Text {
                text: "Cross-fades through the photos in this folder."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Shuffle" }
            Row {
                spacing: 16
                ToggleChip { text: "On"; checked: settings.frameShuffle; onClicked: settings.frameShuffle = true }
                ToggleChip { text: "Off"; checked: !settings.frameShuffle; onClicked: settings.frameShuffle = false }
            }
            Text {
                text: "On: a fresh random order each time Frame Art starts. Off: newest photos first."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }
        }
    }

    Component {
        id: singleContent
        Column {
            spacing: 22
            SectionLabel { text: "Photo" }
            PhotoSwatch {
                selected: settings.frameImagePath && settings.frameImagePath.length > 0
                imagePath: settings.frameImagePath
                onClicked: singlePhotoDialog.open()
            }
            Text {
                text: "Choose a single photo to display."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }
        }
    }

    Component {
        id: clockOptions
        Column {
            spacing: 22
            SectionLabel { text: "Clock position" }
            Flow {
                width: parent.width
                spacing: 12
                ToggleChip { text: "Bottom left"; checked: settings.frameClockPosition === "BottomLeft"; onClicked: settings.frameClockPosition = "BottomLeft" }
                ToggleChip { text: "Bottom center"; checked: settings.frameClockPosition === "BottomCenter"; onClicked: settings.frameClockPosition = "BottomCenter" }
                ToggleChip { text: "Bottom right"; checked: settings.frameClockPosition === "BottomRight"; onClicked: settings.frameClockPosition = "BottomRight" }
                ToggleChip { text: "Center"; checked: settings.frameClockPosition === "Center"; onClicked: settings.frameClockPosition = "Center" }
            }

            SectionLabel { text: "Clock size" }
            Row {
                spacing: 12
                ToggleChip { text: "Small"; checked: settings.frameClockSize === "Small"; onClicked: settings.frameClockSize = "Small" }
                ToggleChip { text: "Medium"; checked: settings.frameClockSize === "Medium"; onClicked: settings.frameClockSize = "Medium" }
                ToggleChip { text: "Large"; checked: settings.frameClockSize === "Large"; onClicked: settings.frameClockSize = "Large" }
            }

            SectionLabel { text: "Date" }
            Row {
                spacing: 16
                ToggleChip { text: "Show"; checked: settings.frameShowDate; onClicked: settings.frameShowDate = true }
                ToggleChip { text: "Hide"; checked: !settings.frameShowDate; onClicked: settings.frameShowDate = false }
            }
        }
    }

    // Dialogs
    FileDialog {
        id: folderDialog
        title: "Select Folder"
        folder: Qt.standardPaths.standardLocations(Qt.PicturesLocation)[0]
        selectFolder: true
        onAccepted: {
            settings.frameFolderName = folderDialog.fileUrl.toString().replace("file://", "")
            settings.frameFolderId = folderDialog.fileUrl.toString()
            settings.frameSource = "Folder"
        }
    }

    FileDialog {
        id: singlePhotoDialog
        title: "Select Photo"
        folder: Qt.standardPaths.standardLocations(Qt.PicturesLocation)[0]
        nameFilters: ["Images (*.png *.jpg *.jpeg *.webp *.bmp)", "All Files (*)"]
        selectExisting: true
        selectMultiple: false
        onAccepted: {
            settings.frameImagePath = singlePhotoDialog.fileUrl.toString().replace("file://", "")
            settings.frameSource = "Single"
        }
    }
}