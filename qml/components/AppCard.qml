import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var appInfo
    property bool isDockItem: false
    property bool glassBlur: false

    signal launch(var appInfo)

    width: isDockItem ? 112 : 240
    height: isDockItem ? 112 : 200

    // Focus handling for controller navigation
    focusPolicy: Qt.StrongFocus
    activeFocusOnTab: true

    // Visual states
    property bool hovered: false
    property bool pressed: false

    // App icon
    Image {
        id: appIcon
        anchors.centerIn: parent
        width: isDockItem ? 80 : 160
        height: isDockItem ? 80 : 160
        fillMode: Image.PreserveAspectFit
        source: appInfo.iconName
            ? "image://appicons/" + appInfo.iconName
            : "qrc:/icons/app-placeholder.svg"
        smooth: true
        mipmap: true
        layer.enabled: hovered || pressed
        layer.effect: DropShadow {
            radius: hovered ? 24 : 12
            samples: hovered ? 16 : 8
            color: "#000000aa"
            verticalOffset: 4
        }
        scale: pressed ? 0.92 : (hovered ? 1.08 : 1.0)
        Behavior on scale {
            SpringAnimation {
                spring: 2
                damping: 0.3
            }
        }
    }

    // App label (only in grid, not dock)
    Text {
        id: appLabel
        anchors.top: appIcon.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: isDockItem ? 0 : 8
        visible: !isDockItem
        text: appInfo.name
        font.pixelSize: 16
        font.weight: Font.Medium
        color: "#ffffff"
        elide: Text.ElideRight
        width: parent.width - 16
        maximumLineCount: 2
        wrapMode: Text.WordWrap
    }

    // Focus indicator
    Rectangle {
        id: focusRing
        anchors.fill: parent
        anchors.margins: -8
        radius: isDockItem ? 20 : 12
        border.color: "#00d4ff"
        border.width: 3
        visible: activeFocus
        opacity: activeFocus ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: 150 }
        }
    }

    // Hover/pressed background for dock items
    Rectangle {
        id: dockBg
        anchors.fill: parent
        anchors.margins: isDockItem ? -4 : 0
        radius: isDockItem ? 20 : 0
        color: pressed ? "#ffffff22" : (hovered ? "#ffffff11" : "transparent")
        visible: isDockItem
    }

    // Mouse/touch area
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        anchors.margins: -8
        hoverEnabled: true
        onEntered: root.hovered = true
        onExited: root.hovered = false
        onPressed: root.pressed = true
        onReleased: root.pressed = false
        onClicked: {
            root.launch(root.appInfo)
        }
    }

    // Keyboard/controller handling
    Keys.onReturnPressed: {
        root.launch(root.appInfo)
        event.accepted = true
    }
    Keys.onEnterPressed: {
        root.launch(root.appInfo)
        event.accepted = true
    }
    Keys.onSpacePressed: {
        root.launch(root.appInfo)
        event.accepted = true
    }
}