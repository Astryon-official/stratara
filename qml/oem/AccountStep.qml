import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var setupData
    property bool canProceed: false
    property bool passwordsMatch: false

    signal canProceedChanged()

    Column {
        anchors.centerIn: parent
        spacing: 24
        width: parent.width

        // Title
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Create Your Account"
            color: "#ffffff"
            font.pixelSize: 36
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "This will be your login for the Stratara system"
            color: "#888888"
            font.pixelSize: 16
        }

        // Form
        Column {
            spacing: 20
            width: Math.min(parent.width, 500)
            anchors.horizontalCenter: parent.horizontalCenter

            // Username
            Column {
                spacing: 8
                Text {
                    text: "Username"
                    color: "#aaaaaa"
                    font.pixelSize: 14
                }
                TextField {
                    id: usernameField
                    width: parent.width
                    placeholderText: "Enter username (e.g. stratara)"
                    text: setupData.username
                    onTextChanged: {
                        setupData.username = text
                        validateForm()
                    }
                    background: Rectangle {
                        radius: 8
                        color: "#1e1e2e"
                        border.color: usernameField.focus ? "#00d4aa" : "#333333"
                        border.width: usernameField.focus ? 2 : 1
                    }
                }
                Text {
                    text: usernameField.text.length < 3 ? "Username must be at least 3 characters" : ""
                    color: usernameField.text.length > 0 && usernameField.text.length < 3 ? "#ff6b6b" : "transparent"
                    font.pixelSize: 12
                }
            }

            // Password
            Column {
                spacing: 8
                Text {
                    text: "Password"
                    color: "#aaaaaa"
                    font.pixelSize: 14
                }
                TextField {
                    id: passwordField
                    width: parent.width
                    placeholderText: "Enter password"
                    echoMode: TextField.Password
                    onTextChanged: {
                        setupData.password = text
                        validateForm()
                    }
                    background: Rectangle {
                        radius: 8
                        color: "#1e1e2e"
                        border.color: passwordField.focus ? "#00d4aa" : "#333333"
                        border.width: passwordField.focus ? 2 : 1
                    }
                }
                // Password strength indicator
                Item {
                    height: 8
                    width: parent.width
                    Rectangle {
                        anchors.fill: parent
                        radius: 4
                        color: "#333333"
                        Rectangle {
                            id: strengthBar
                            height: parent.height
                            radius: 4
                            color: passwordStrengthColor
                            width: parent.width * passwordStrength
                        }
                    }
                }
                Text {
                    text: passwordStrengthText
                    color: passwordStrengthColor
                    font.pixelSize: 12
                }
            }

            // Confirm Password
            Column {
                spacing: 8
                Text {
                    text: "Confirm Password"
                    color: "#aaaaaa"
                    font.pixelSize: 14
                }
                TextField {
                    id: confirmField
                    width: parent.width
                    placeholderText: "Confirm password"
                    echoMode: TextField.Password
                    onTextChanged: {
                        root.passwordsMatch = text === passwordField.text
                        validateForm()
                    }
                    background: Rectangle {
                        radius: 8
                        color: "#1e1e2e"
                        border.color: confirmField.focus ? (root.passwordsMatch ? "#00d4aa" : "#ff6b6b") : "#333333"
                        border.width: confirmField.focus ? 2 : 1
                    }
                }
                Text {
                    text: confirmField.text.length > 0 && !root.passwordsMatch ? "Passwords do not match" : ""
                    color: "#ff6b6b"
                    font.pixelSize: 12
                }
            }

            // Auto-login option
            Row {
                spacing: 12
                CheckBox {
                    id: autoLoginCheck
                    checked: true
                    indicator: Rectangle {
                        width: 24
                        height: 24
                        radius: 6
                        color: checked ? "#00d4aa" : "#1e1e2e"
                        border.color: checked ? "#00d4aa" : "#333333"
                        border.width: 2
                    }
                }
                Text {
                    text: "Auto-login on boot (recommended for living room)"
                    color: "#ffffff"
                    font.pixelSize: 15
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        // Help text
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Use a simple password you'll remember. This is a local account only."
            color: "#666666"
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }
    }

    // Computed properties
    readonly property real passwordStrength: {
        var len = passwordField.text.length
        var hasUpper = /[A-Z]/.test(passwordField.text)
        var hasLower = /[a-z]/.test(passwordField.text)
        var hasNumber = /[0-9]/.test(passwordField.text)
        var hasSymbol = /[^A-Za-z0-9]/.test(passwordField.text)
        var score = 0
        if (len >= 8) score += 0.25
        if (hasUpper) score += 0.25
        if (hasLower) score += 0.25
        if (hasNumber) score += 0.125
        if (hasSymbol) score += 0.125
        return Math.min(score, 1.0)
    }

    readonly property string passwordStrengthColor: {
        var s = passwordStrength
        if (s < 0.3) return "#ff6b6b"
        if (s < 0.6) return "#ffcc00"
        return "#00d4aa"
    }

    readonly property string passwordStrengthText: {
        var s = passwordStrength
        if (s < 0.3) return "Weak"
        if (s < 0.6) return "Fair"
        if (s < 0.8) return "Good"
        return "Strong"
    }

    function validateForm() {
        var valid = usernameField.text.length >= 3 &&
                    passwordField.text.length >= 8 &&
                    root.passwordsMatch
        root.canProceed = valid
        root.canProceedChanged()
    }

    Component.onCompleted: {
        validateForm()
    }
}