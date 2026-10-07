import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: root
    property Settings settings

    anchors.fill: parent
    focus: true

    property bool showCityPicker: false

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

            PaneTitle { text: "Weather" }
            Text {
                text: "A quiet temperature and condition readout. Show it on the home screen, on Frame Art, or both."
                color: "#aaaaaa"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            SectionLabel { text: "Display" }
            Row {
                spacing: 16
                ToggleChip { text: "Show on Home"; checked: settings.weatherOnHome; onClicked: settings.weatherOnHome = true }
                ToggleChip { text: "Show in Frame Art"; checked: settings.frameWeather; onClicked: settings.frameWeather = true }
            }

            SectionLabel { text: "Temperature Unit" }
            Row {
                spacing: 16
                ToggleChip { text: "Celsius"; checked: settings.weatherUnit === "Celsius"; onClicked: settings.weatherUnit = "Celsius" }
                ToggleChip { text: "Fahrenheit"; checked: settings.weatherUnit === "Fahrenheit"; onClicked: settings.weatherUnit = "Fahrenheit" }
            }

            SectionLabel { text: "Location" }
            Column {
                spacing: 12
                Row {
                    spacing: 16
                    ToggleChip {
                        text: settings.weatherCity && settings.weatherCity.length > 0 ? settings.weatherCity : "Auto (by IP)"
                        checked: !settings.weatherCity || settings.weatherCity.length === 0
                        onClicked: {
                            settings.weatherCity = ""
                            settings.weatherLat = 0
                            settings.weatherLon = 0
                        }
                    }
                }
                Text {
                    text: settings.weatherCity && settings.weatherCity.length > 0 ? "Weather is pinned to this city." : "Weather detects your location automatically by IP."
                    color: "#aaaaaa"
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    width: parent.width * 0.85
                }
                Row {
                    spacing: 16
                    ToggleChip { text: "Choose city"; checked: false; onClicked: cityDialog.open() }
                    ToggleChip { text: "Auto-detect"; checked: !settings.weatherCity || settings.weatherCity.length === 0; onClicked: {
                        settings.weatherCity = ""
                        settings.weatherLat = 0
                        settings.weatherLon = 0
                    }}
                }
            }
        }
    }

    FileDialog {
        id: cityDialog
        title: "Select City"
        // Note: In a real implementation, this would be a city picker dialog
        // For now, we just show a text field
        onAccepted: {
            // TODO: Implement city picker with geocoding
        }
    }
}