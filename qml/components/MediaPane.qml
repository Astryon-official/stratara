import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property MediaManager mediaManager

    Column {
        anchors.fill: parent
        spacing: 24
        anchors.margins: 32

        // Active player
        SettingsSection {
            title: "Now Playing"
            spacing: 16

            Rectangle {
                visible: mediaManager.hasActivePlayer
                Layout.fillWidth: true
                height: 140
                radius: 16
                color: "#1a1a2e"
                border.color: "#ffffff22"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 16
                    anchors.margins: 16

                    Rectangle {
                        width: 100
                        height: 100
                        radius: 12
                        color: "#2a2a2c"
                        border.color: "#ffffff22"
                        border.width: 1

                        Image {
                            anchors.fill: parent
                            source: mediaManager.artUrl
                            fillMode: Image.PreserveAspectCrop
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4

                        Label {
                            text: mediaManager.title
                            font.pixelSize: 20
                            font.weight: Font.Medium
                            color: "#ffffff"
                            elide: Text.ElideRight
                        }

                        Label {
                            text: mediaManager.artist
                            font.pixelSize: 16
                            color: "#aaaaaa"
                            elide: Text.ElideRight
                        }

                        Label {
                            text: mediaManager.album
                            font.pixelSize: 14
                            color: "#888888"
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            Label {
                visible: !mediaManager.hasActivePlayer
                text: "No active media player"
                font.pixelSize: 16
                color: "#aaaaaa"
            }

            // Playback controls
            Rectangle {
                visible: mediaManager.hasActivePlayer
                Layout.fillWidth: true
                height: 80
                radius: 12
                color: "#1a1a2e"
                border.color: "#ffffff22"
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 12

                    // Progress bar
                    Slider {
                        id: progressSlider
                        Layout.fillWidth: true
                        from: 0
                        to: mediaManager.duration > 0 ? mediaManager.duration / 1000000 : 100
                        value: mediaManager.position > 0 ? mediaManager.position / 1000000 : 0
                        stepSize: 1
                        onValueChanged: {
                            if (!pressed) mediaManager.seek(value * 1000000)
                        }
                    }

                    Row {
                        Layout.fillWidth: true
                        spacing: 16

                        Label {
                            text: formatTime(mediaManager.position / 1000000)
                            font.pixelSize: 12
                            color: "#888888"
                        }

                        Item { Layout.fillWidth: true }

                        Label {
                            text: mediaManager.duration > 0 ? formatTime(mediaManager.duration / 1000000) : "--:--"
                            font.pixelSize: 12
                            color: "#888888"
                        }
                    }

                    Row {
                        Layout.fillWidth: true
                        spacing: 24
                        anchors.margins: Qt.size(0, 8, 0, 0)

                        Button {
                            text: "⏮"
                            font.pixelSize: 24
                            background: Rectangle { radius: 24; color: "transparent" }
                            onClicked: mediaManager.previous()
                        }

                        Button {
                            text: mediaManager.playbackStatus === "Playing" ? "⏸" : "▶"
                            font.pixelSize: 28
                            background: Rectangle { radius: 28; color: "#44aa44" }
                            onClicked: mediaManager.playPause()
                        }

                        Button {
                            text: "⏭"
                            font.pixelSize: 24
                            background: Rectangle { radius: 24; color: "transparent" }
                            onClicked: mediaManager.next()
                        }

                        Item { Layout.fillWidth: true }

                        // Volume slider
                        Column {
                            width: 120
                            spacing: 4

                            Row {
                                spacing: 8
                                Image {
                                    source: "qrc:/icons/wifi-warning.svg"
                                    width: 16
                                    height: 16
                                    color: "#ffffff"
                                }
                                Slider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 1
                                    value: mediaManager.volume
                                    stepSize: 0.05
                                    onValueChanged: {
                                        if (!pressed) mediaManager.setVolume(value)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Shuffle and loop
            Rectangle {
                visible: mediaManager.hasActivePlayer
                Layout.fillWidth: true
                height: 60
                radius: 12
                color: "#1a1a2e"
                border.color: "#ffffff22"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 24
                    anchors.margins: 16

                    Button {
                        text: "🔀"
                        font.pixelSize: 20
                        checkable: true
                        checked: mediaManager.shuffle
                        background: Rectangle {
                            radius: 24
                            color: checked ? "#44aa44" : "transparent"
                            border.color: checked ? "#44aa44" : "#ffffff22"
                            border.width: 1
                        }
                        onClicked: mediaManager.setShuffle(checked)
                    }

                    Button {
                        text: loopIcon(mediaManager.loopStatus)
                        font.pixelSize: 20
                        checkable: true
                        checked: mediaManager.loopStatus !== 0
                        background: Rectangle {
                            radius: 24
                            color: checked ? "#44aa44" : "transparent"
                            border.color: checked ? "#44aa44" : "#ffffff22"
                            border.width: 1
                        }
                        onClicked: mediaManager.setLoopStatus(checked ? (mediaManager.loopStatus === 1 ? 2 : 0) : 1)
                    }
                }
            }
        }

        // Available players
        SettingsSection {
            title: "Available Players"
            spacing: 16

            ListView {
                Layout.fillWidth: true
                height: 200
                model: mediaManager.players
                spacing: 8
                delegate: ItemDelegate {
                    width: parent.width
                    height: 60
                    contentItem: Row {
                        anchors.fill: parent
                        spacing: 16

                        Rectangle {
                            width: 40
                            height: 40
                            radius: 20
                            color: model.status !== "Stopped" ? "#44aa44" : "#2a2a2c"
                            border.color: "#ffffff22"
                            border.width: 1
                            Image {
                                anchors.centerIn: parent
                                source: "qrc:/icons/wifi-warning.svg"
                                width: 20
                                height: 20
                                color: model.status !== "Stopped" ? "#ffffff" : "#888888"
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Label {
                                text: model.identity !== undefined ? model.identity : model.name
                                font.pixelSize: 18
                                font.weight: Font.Medium
                                color: "#ffffff"
                            }
                            Label {
                                text: model.status
                                font.pixelSize: 13
                                color: "#888888"
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            visible: model.status !== "Stopped"
                            text: "Switch"
                            onClicked: mediaManager.setActivePlayer(model.name)
                        }
                    }
                }
            }
        }
    }

    function formatTime(seconds) {
        var mins = Math.floor(seconds / 60)
        var secs = Math.floor(seconds % 60)
        return mins + ":" + (secs < 10 ? "0" : "") + secs
    }

    function loopIcon(status) {
        switch (status) {
            case 0: return "🔁" // None
            case 1: return "🔂" // Track
            case 2: return "🔁" // Playlist
            default: return "🔁"
        }
    }
}