import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: root
    property Settings settings
    property AppModel appModel
    property FavoritesStore favoritesStore
    property UpdateStore updateStore
    property UpdateChecker updateChecker
    property UpdateDownloader updateDownloader
    property UpdateInstaller updateInstaller
    property NetworkManager networkManager
    property BluetoothManager bluetoothManager
    property PowerManager powerManager
    property MediaManager mediaManager

    signal closed()

    anchors.fill: parent
    visible: false
    focus: true

    // State
    property var currentSection: "Appearance"
    property var holdUpdatesSection: false
    property var startAtUpdates: false

    enum Section {
        "Appearance",
        "Frame Art",
        "Weather",
        "Home Setup",
        "Hidden Apps",
        "Updates",
        "Diagnostics",
        "Network",
        "Bluetooth",
        "Power",
        "Media"
    }

    // Modal backdrop
    Rectangle {
        id: modalBg
        anchors.fill: parent
        color: "#000000cc"
        MouseArea {
            anchors.fill: parent
            onClicked: root.close()
        }
    }

    // Settings panel
    Rectangle {
        id: panel
        width: Math.min(1000, parent.width - 80)
        height: Math.min(900, parent.height - 80)
        anchors.centerIn: parent
        radius: 16
        color: "#1a1a2e"
        border.color: "#ffffff22"
        border.width: 1

        Row {
            anchors.fill: parent
            spacing: 0

            // Left: section navigation rail
            Rectangle {
                id: navRail
                width: 280
                color: "#16213e"
                border.color: "#ffffff11"
                border.width: 1

                Column {
                    anchors.fill: parent
                    spacing: 0

                    // Title
                    Label {
                        text: "Settings"
                        font.pixelSize: 34
                        font.weight: Font.Bold
                        color: "#ffffff"
                        anchors.margins: 24
                    }

                    Spacer {
                        height: 28
                    }

                    // Section buttons
                    Column {
                        id: sectionList
                        spacing: 6

                        SectionNavButton {
                            id: appearanceBtn
                            title: "Appearance"
                            active: root.currentSection === "Appearance"
                            focusRequester: firstSectionRequester
                            onClicked: root.currentSection = "Appearance"
                        }
                        SectionNavButton {
                            id: frameArtBtn
                            title: "Frame Art"
                            active: root.currentSection === "Frame Art"
                            onClicked: root.currentSection = "Frame Art"
                        }
                        SectionNavButton {
                            id: weatherBtn
                            title: "Weather"
                            active: root.currentSection === "Weather"
                            onClicked: root.currentSection = "Weather"
                        }
                        SectionNavButton {
                            id: homeSetupBtn
                            title: "Home Setup"
                            active: root.currentSection === "Home Setup"
                            onClicked: root.currentSection = "Home Setup"
                        }
                        SectionNavButton {
                            id: hiddenAppsBtn
                            title: "Hidden Apps"
                            active: root.currentSection === "Hidden Apps"
                            onClicked: root.currentSection = "Hidden Apps"
                        }
                        SectionNavButton {
                            id: updatesBtn
                            title: "Updates"
                            active: root.currentSection === "Updates"
                            focusRequester: updatesSectionRequester
                            onClicked: {
                                root.currentSection = "Updates"
                            }
                        }
                        SectionNavButton {
                            id: diagnosticsBtn
                            title: "Diagnostics"
                            active: root.currentSection === "Diagnostics"
                            onClicked: root.currentSection = "Diagnostics"
                        }
                        SectionNavButton {
                            id: networkBtn
                            title: "Network"
                            active: root.currentSection === "Network"
                            onClicked: root.currentSection = "Network"
                        }
                        SectionNavButton {
                            id: bluetoothBtn
                            title: "Bluetooth"
                            active: root.currentSection === "Bluetooth"
                            onClicked: root.currentSection = "Bluetooth"
                        }
                        SectionNavButton {
                            id: powerBtn
                            title: "Power"
                            active: root.currentSection === "Power"
                            onClicked: root.currentSection = "Power"
                        }
                        SectionNavButton {
                            id: mediaBtn
                            title: "Media"
                            active: root.currentSection === "Media"
                            onClicked: root.currentSection = "Media"
                        }
                    }
                }
            }

            // Right: detail pane
            Rectangle {
                id: detailPane
                Layout.fillWidth: true
                color: "transparent"

                // Focus properties to prevent leaking focus to nav rail
                focus: true
                FocusScope {
                    anchors.fill: parent
                    focus: true

                    Loader {
                        id: sectionLoader
                        anchors.fill: parent
                        sourceComponent: sectionComponents[root.currentSection]
                        active: true
                    }
                }
            }
        }
    }

    // Focus requesters
    FocusRequester { id: firstSectionRequester }
    FocusRequester { id: updatesSectionRequester }

    // Section component mapping
    property var sectionComponents: {
        "Appearance": appearancePane,
        "Frame Art": frameArtPane,
        "Weather": weatherPane,
        "Home Setup": homeSetupPane,
        "Hidden Apps": hiddenAppsPane,
        "Updates": updatesPane,
        "Diagnostics": diagnosticsPane,
        "Network": networkPane,
        "Bluetooth": bluetoothPane,
        "Power": powerPane,
        "Media": mediaPane
    }

    // Open on Updates when startAtUpdates is true
    Component.onCompleted: {
        if (startAtUpdates) {
            currentSection = "Updates"
        }
        firstSectionRequester.requestFocus()
    }

    // Hold Updates section during transient flow
    Connections {
        target: updateChecker
        onCheckFinished: {
            if (result.type === UpdateResult.UpdateAvailable || result.type === UpdateResult.Downloading) {
                holdUpdatesSection = true
                currentSection = "Updates"
            } else if (holdUpdatesSection) {
                // Brief hold to let action chip reclaim focus
                setTimeout(() => {
                    holdUpdatesSection = false
                }, 300)
            }
        }
    }

    // Section panes
    Component {
        id: appearancePane
        AppearancePane {
            settings: root.settings
            favoritesStore: root.favoritesStore
            appModel: root.appModel
        }
    }

    Component {
        id: frameArtPane
        FrameArtPane {
            settings: root.settings
        }
    }

    Component {
        id: weatherPane
        WeatherPane {
            settings: root.settings
        }
    }

    Component {
        id: homeSetupPane
        HomeSetupPane {
            settings: root.settings
            appModel: root.appModel
        }
    }

    Component {
        id: hiddenAppsPane
        HiddenAppsPane {
            settings: root.settings
            appModel: root.appModel
        }
    }

    Component {
        id: updatesPane
        UpdatesPane {
            updateStore: root.updateStore
            updateChecker: root.updateChecker
            updateDownloader: root.updateDownloader
            updateInstaller: root.updateInstaller
            railFocus: updatesSectionRequester
        }
    }

    Component {
        id: diagnosticsPane
        DiagnosticsPane {
            settings: root.settings
        }
    }

    Component {
        id: networkPane
        NetworkPane {
            settings: root.settings
            networkManager: root.networkManager
        }
    }

    Component {
        id: bluetoothPane
        BluetoothPane {
            settings: root.settings
            bluetoothManager: root.bluetoothManager
        }
    }

    Component {
        id: powerPane
        PowerPane {
            settings: root.settings
            powerManager: root.powerManager
        }
    }

    Component {
        id: mediaPane
        MediaPane {
            settings: root.settings
            mediaManager: root.mediaManager
        }
    }

    function close() {
        visible = false
        root.closed()
    }

    Keys.onEscapePressed: close()
}

// Section navigation button
Component {
    id: sectionNavButtonComponent
    Button {
        property string title
        property bool active: false
        property var focusRequester: null
        width: parent.width
        height: 56
        background: Rectangle {
            anchors.fill: parent
            color: active ? "#ffffff1a" : "transparent"
            border.color: "#ffffff11"
            border.width: active ? 2 : 0
        }
        contentItem: Text {
            text: title
            font.pixelSize: 19
            font.weight: Font.Medium
            color: active ? "#ffffff" : "#aaaaaa"
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 20
        }
        focusPolicy: Qt.StrongFocus
        FocusRequester { id: focusRequester }
        onFocusChanged: {
            if (focus) {
                if (focusRequester) focusRequester.requestFocus()
            }
        }
    }
}