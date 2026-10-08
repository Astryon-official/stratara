import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import Stratara.OemSetup

Window {
    id: root
    width: 1920
    height: 1080
    visible: true
    title: "Stratara Setup"
    color: "#000000"
    flags: Qt.Window | Qt.FramelessWindowHint

    property int currentStep: 0
    property var setupData: {
        "hostname": "",
        "username": "",
        "password": "",
        "timezone": "UTC",
        "locale": "en_US.UTF-8",
        "keyboardLayout": "us",
        "wifiSsid": "",
        "wifiPassword": "",
        "useEthernet": false,
        "enrollXodus": true,
        "xodusName": "Stratara Living Room",
        "xodusType": "living-room-shell",
        "displayMode": "1920x1080",
        "calibrated": false
    }

    Component.onCompleted: {
        fullScreen = true
    }

    // Background
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0a0a0c" }
            GradientStop { position: 0.5; color: "#1a1a2e" }
            GradientStop { position: 1.0; color: "#16213e" }
        }
    }

    // Stratara logo watermark
    Image {
        anchors.centerIn: parent
        source: "qrc:/icons/stratara.svg"
        width: 400
        height: 400
        opacity: 0.03
        fillMode: Image.PreserveAspectFit
    }

    // Step indicator at top
    Row {
        id: stepIndicator
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 48
        spacing: 16

        Repeater {
            model: 5
            delegate: Item {
                width: 40
                height: 40
                Rectangle {
                    id: stepCircle
                    anchors.centerIn: parent
                    width: 32
                    height: 32
                    radius: 16
                    color: index < root.currentStep ? "#00d4aa" : (index === root.currentStep ? "#ffffff" : "#333333")
                    border.color: index === root.currentStep ? "#00d4aa" : "transparent"
                    border.width: 2

                    Text {
                        anchors.centerIn: parent
                        text: index + 1
                        color: index <= root.currentStep ? "#000000" : "#888888"
                        font.pixelSize: 14
                        font.bold: true
                    }
                }
                Text {
                    anchors.top: stepCircle.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.margins: 8
                    text: ["Welcome", "Network", "Account", "Xodus", "Display"][index]
                    color: index <= root.currentStep ? "#ffffff" : "#666666"
                    font.pixelSize: 11
                    font.bold: index === root.currentStep
                }
            }
        }
    }

    // Main content area
    Loader {
        id: stepLoader
        anchors.fill: parent
        anchors.topMargin: 140
        anchors.bottomMargin: 100
        anchors.leftMargin: 100
        anchors.rightMargin: 100
        sourceComponent: welcomeStep
        asynchronous: true
    }

    // Navigation buttons
    Row {
        id: navButtons
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 48
        spacing: 24

        Button {
            id: backButton
            text: "Back"
            enabled: root.currentStep > 0
            onClicked: {
                root.currentStep--
            }
            Layout.preferredWidth: 140
            Layout.preferredHeight: 48
            font.pixelSize: 16
            Material.background: "#1e1e2e"
            Material.foreground: "#ffffff"
        }

        Button {
            id: nextButton
            text: root.currentStep === 4 ? "Finish" : "Next"
            enabled: stepLoader.item && stepLoader.item.canProceed
            onClicked: {
                if (root.currentStep < 4) {
                    root.currentStep++
                } else {
                    // Finish - run setup
                    oemRunner.runSetup()
                }
            }
            Layout.preferredWidth: 140
            Layout.preferredHeight: 48
            font.pixelSize: 16
            Material.background: "#00d4aa"
            Material.foreground: "#000000"
        }
    }

    // Step components
    Component {
        id: welcomeStep
        WelcomeStep {
            onCanProceedChanged: nextButton.enabled = canProceed
        }
    }

    Component {
        id: networkStep
        NetworkStep {
            setupData: root.setupData
            onCanProceedChanged: nextButton.enabled = canProceed
        }
    }

    Component {
        id: accountStep
        AccountStep {
            setupData: root.setupData
            onCanProceedChanged: nextButton.enabled = canProceed
        }
    }

    Component {
        id: xodusStep
        XodusStep {
            setupData: root.setupData
            onCanProceedChanged: nextButton.enabled = canProceed
        }
    }

    Component {
        id: displayStep
        DisplayStep {
            setupData: root.setupData
            onCanProceedChanged: nextButton.enabled = canProceed
        }
    }

    // OEM runner for executing the setup
    OemRunner {
        id: oemRunner
        setupData: root.setupData
        onSetupComplete: {
            Qt.quit()
        }
        onSetupError: {
            errorDialog.title = "Setup Error"
            errorDialog.text = message
            errorDialog.open()
        }
    }

    // Error dialog
    MessageDialog {
        id: errorDialog
        standardButtons: MessageDialog.Ok
    }

    // Step change handler
    onCurrentStepChanged: {
        switch (currentStep) {
        case 0: stepLoader.sourceComponent = welcomeStep; break
        case 1: stepLoader.sourceComponent = networkStep; break
        case 2: stepLoader.sourceComponent = accountStep; break
        case 3: stepLoader.sourceComponent = xodusStep; break
        case 4: stepLoader.sourceComponent = displayStep; break
        }
    }
}