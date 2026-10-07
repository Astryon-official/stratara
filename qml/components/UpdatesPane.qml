import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property UpdateStore updateStore
    property UpdateChecker updateChecker
    property UpdateDownloader updateDownloader
    property UpdateInstaller updateInstaller
    property var railFocus

    anchors.fill: parent
    focus: true

    property var updateState: UpdateStore.Idle

    Connections {
        target: updateChecker
        onCheckFinished: {
            root.updateState = result
        }
    }

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

            PaneTitle { text: "Updates" }

            // Current status
            Item {
                width: parent.width
                Column {
                    spacing: 16

                    // Version info
                    Row {
                        spacing: 16
                        Label {
                            text: "Current Version"
                            font.pixelSize: 16
                            color: "#ffffff"
                            verticalAlignment: Text.AlignVCenter
                        }
                        Label {
                            text: Qt.applicationVersion
                            font.pixelSize: 16
                            font.weight: Font.Medium
                            color: "#00d4ff"
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    // Last checked
                    Row {
                        spacing: 16
                        Label {
                            text: "Last Checked"
                            font.pixelSize: 16
                            color: "#ffffff"
                            verticalAlignment: Text.AlignVCenter
                        }
                        Label {
                            text: updateStore.lastCheckedAtMillis > 0
                                ? Qt.formatDateTime(new Date(updateStore.lastCheckedAtMillis), "MMM d, yyyy hh:mm AP")
                                : "Never"
                            font.pixelSize: 16
                            color: "#aaaaaa"
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Action buttons based on state
            Loader {
                sourceComponent: stateComponents[updateState.type]
            }
        }
    }

    property var stateComponents: {
        "Idle": idleContent,
        "Checking": checkingContent,
        "UpToDate": upToDateContent,
        "UpdateAvailable": updateAvailableContent,
        "Downloading": downloadingContent,
        "ReadyToInstall": readyToInstallContent,
        "NeedsInstallPermission": needsPermissionContent,
        "SignatureMismatch": signatureMismatchContent,
        "Error": errorContent
    }

    Component {
        id: idleContent
        Row {
            spacing: 16
            Button {
                text: "Check for Updates"
                width: 200
                background: Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "#00d4ff"
                }
                contentItem: Text {
                    text: "Check for Updates"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "#000000"
                    anchors.centerIn: parent
                }
                onClicked: updateChecker.checkForUpdate(Date.now(), false)
            }
            Button {
                text: "Force Check"
                width: 140
                background: Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "#ffffff22"
                    border.color: "#ffffff44"
                    border.width: 1
                }
                contentItem: Text {
                    text: "Force Check"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "#ffffff"
                    anchors.centerIn: parent
                }
                onClicked: updateChecker.checkForUpdate(Date.now(), true)
            }
        }
    }

    Component {
        id: checkingContent
        Row {
            spacing: 16
            BusyIndicator {
                running: true
                width: 24
                height: 24
            }
            Label {
                text: "Checking for updates..."
                font.pixelSize: 16
                color: "#ffffff"
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Component {
        id: upToDateContent
        Column {
            spacing: 12
            Row {
                spacing: 12
                Image {
                    source: "qrc:/icons/check.svg"
                    width: 24
                    height: 24
                    color: "#00ff00"
                }
                Label {
                    text: "You're up to date"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: "#00ff00"
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text {
                text: "Stratara is running the latest version."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }
            Row {
                spacing: 16
                Button {
                    text: "Check Again"
                    width: 160
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: "#00d4ff"
                    }
                    contentItem: Text {
                        text: "Check Again"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#000000"
                        anchors.centerIn: parent
                    }
                    onClicked: updateChecker.checkForUpdate(Date.now(), true)
                }
            }
        }
    }

    Component {
        id: updateAvailableContent
        Column {
            spacing: 16
            Row {
                spacing: 12
                Image {
                    source: "qrc:/icons/download.svg"
                    width: 24
                    height: 24
                    color: "#ffaa00"
                }
                Column {
                    Label {
                        text: "Version " + updateState.versionTag + " available"
                        font.pixelSize: 18
                        font.weight: Font.Medium
                        color: "#ffaa00"
                    }
                    Text {
                        text: updateState.changelog.length > 0 ? updateState.changelog : "No changelog provided."
                        color: "#aaaaaa"
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        width: parent.width * 0.85
                    }
                }
            }
            Row {
                spacing: 16
                Button {
                    text: "Download Update"
                    width: 180
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: "#00d4ff"
                    }
                    contentItem: Text {
                        text: "Download Update"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#000000"
                        anchors.centerIn: parent
                    }
                    onClicked: {
                        updateDownloader.startDownload(updateState.downloadUrl)
                    }
                }
                Button {
                    text: "Later"
                    width: 100
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: "#ffffff22"
                        border.color: "#ffffff44"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: "Later"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#ffffff"
                        anchors.centerIn: parent
                    }
                    onClicked: {
                        updateStore.dismissNotice(updateState.versionTag)
                    }
                }
            }
        }
    }

    Component {
        id: downloadingContent
        Column {
            spacing: 16
            Row {
                spacing: 12
                BusyIndicator {
                    running: true
                    width: 24
                    height: 24
                }
                Label {
                    text: "Downloading update..."
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: "#ffaa00"
                    verticalAlignment: Text.AlignVCenter
                }
            }
            ProgressBar {
                width: parent.width
                height: 8
                value: updateDownloader.progressPercent / 100.0
                background: Rectangle {
                    radius: 4
                    color: "#ffffff22"
                }
                contentItem: Rectangle {
                    radius: 4
                    color: "#00d4ff"
                }
            }
            Row {
                spacing: 16
                Label {
                    text: "Progress: " + updateDownloader.progressPercent + "%"
                    font.pixelSize: 14
                    color: "#ffffff"
                }
                Label {
                    text: formatBytes(updateDownloader.bytesDownloaded) + " / " + formatBytes(updateDownloader.bytesTotal)
                    font.pixelSize: 13
                    color: "#aaaaaa"
                }
            }
            function formatBytes(bytes) {
                if (bytes < 1024) return bytes + " B"
                else if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + " KB"
                else return (bytes / (1024 * 1024)).toFixed(1) + " MB"
            }
            Button {
                text: "Cancel"
                width: 100
                background: Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "#ff4444"
                }
                contentItem: Text {
                    text: "Cancel"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    color: "#ffffff"
                    anchors.centerIn: parent
                }
                onClicked: updateDownloader.cancelDownload()
            }
        }
    }

    Component {
        id: readyToInstallContent
        Column {
            spacing: 16
            Row {
                spacing: 12
                Image {
                    source: "qrc:/icons/check.svg"
                    width: 24
                    height: 24
                    color: "#00d4ff"
                }
                Label {
                    text: "Download complete. Ready to install."
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: "#00d4ff"
                }
            }
            Row {
                spacing: 16
                Button {
                    text: "Install Now"
                    width: 160
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: "#00d4ff"
                    }
                    contentItem: Text {
                        text: "Install Now"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#000000"
                        anchors.centerIn: parent
                    }
                    onClicked: {
                        var result = updateInstaller.install(updateDownloader.downloadPath)
                        if (result.type === InstallResult.NeedsPermission) {
                            // State will update via signal
                        }
                    }
                }
            }
        }
    }

    Component {
        id: needsPermissionContent
        Column {
            spacing: 16
            Row {
                spacing: 12
                Image {
                    source: "qrc:/icons/alert.svg"
                    width: 24
                    height: 24
                    color: "#ffaa00"
                }
                Label {
                    text: "Install permission required"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: "#ffaa00"
                }
            }
            Text {
                text: "Stratara needs permission to install packages. This is handled by the system package manager (polkit)."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }
            Row {
                spacing: 16
                Button {
                    text: "Grant Permission"
                    width: 180
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: "#00d4ff"
                    }
                    contentItem: Text {
                        text: "Grant Permission"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#000000"
                        anchors.centerIn: parent
                    }
                    onClicked: updateInstaller.requestInstallPermission()
                }
            }
        }
    }

    Component {
        id: signatureMismatchContent
        Column {
            spacing: 16
            Row {
                spacing: 12
                Image {
                    source: "qrc:/icons/close.svg"
                    width: 24
                    height: 24
                    color: "#ff4444"
                }
                Label {
                    text: "Signature mismatch"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: "#ff4444"
                }
            }
            Text {
                text: "The downloaded update is not signed with the same certificate as the current installation. This could indicate a tampered or unofficial build."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }
            Row {
                spacing: 16
                Button {
                    text: "Dismiss"
                    width: 100
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: "#ffffff22"
                        border.color: "#ffffff44"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: "Dismiss"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#ffffff"
                        anchors.centerIn: parent
                    }
                    onClicked: updateStore.clearPendingUpdate()
                }
            }
        }
    }

    Component {
        id: errorContent
        Column {
            spacing: 16
            Row {
                spacing: 12
                Image {
                    source: "qrc:/icons/alert.svg"
                    width: 24
                    height: 24
                    color: "#ff4444"
                }
                Label {
                    text: "Update check failed"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    color: "#ff4444"
                }
            }
            Text {
                text: updateState.errorMessage || "An unknown error occurred."
                color: "#aaaaaa"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }
            Row {
                spacing: 16
                Button {
                    text: "Retry"
                    width: 100
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: "#00d4ff"
                    }
                    contentItem: Text {
                        text: "Retry"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#000000"
                        anchors.centerIn: parent
                    }
                    onClicked: updateChecker.checkForUpdate(Date.now(), true)
                }
                Button {
                    text: "Dismiss"
                    width: 100
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: "#ffffff22"
                        border.color: "#ffffff44"
                        border.width: 1
                    }
                    contentItem: Text {
                        text: "Dismiss"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: "#ffffff"
                        anchors.centerIn: parent
                    }
                    onClicked: updateStore.clearPendingUpdate()
                }
            }
        }
    }
}