import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
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

            PaneTitle { text: "Hidden Apps" }
            Text {
                text: "Select apps to hide from the grid. Hidden apps can still be launched but won't appear in the app grid."
                color: "#aaaaaa"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                width: parent.width * 0.85
            }

            ListView {
                width: parent.width
                height: parent.height - 200
                model: appModel
                delegate: HiddenAppDelegate {
                    width: parent.width
                    height: 56
                    appInfo: model
                    hidden: settings.hiddenApps.indexOf(model.packageName) >= 0
                    onToggled: {
                        var hiddenApps = settings.hiddenApps
                        if (hidden) {
                            hiddenApps = hiddenApps.filter(function(pkg) { return pkg !== model.packageName })
                        } else {
                            hiddenApps.push(model.packageName)
                        }
                        settings.hiddenApps = hiddenApps
                    }
                }
            }
        }
    }
}