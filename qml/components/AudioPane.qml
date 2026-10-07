import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property Settings settings
    property PipeWireManager pipeWireManager

    Column {
        anchors.fill: parent
        spacing: 24
        anchors.margins: 32

        // PipeWire status
        SettingsSection {
            title: "PipeWire Audio System"
            spacing: 16

            Row {
                Layout.fillWidth: true
                spacing: 16

                Column {
                    spacing: 4
                    Label {
                        text: "PipeWire"
                        font.pixelSize: 20
                        font.weight: Font.Medium
                        color: "#ffffff"
                    }
                    Label {
                        text: pipeWireManager.available ? "Available" : "Not Available"
                        font.pixelSize: 14
                        color: pipeWireManager.available ? "#44aa44" : "#ff4444"
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Refresh"
                    onClicked: pipeWireManager.refreshDevices()
                }
            }
        }

        // Audio Output Devices (Sinks)
        SettingsSection {
            title: "Output Devices"
            spacing: 16

            ListView {
                Layout.fillWidth: true
                height: 250
                model: pipeWireManager.audioSinks
                spacing: 8
                delegate: ItemDelegate {
                    width: parent.width
                    height: 72
                    contentItem: Row {
                        anchors.fill: parent
                        spacing: 16

                        Rectangle {
                            width: 48
                            height: 48
                            radius: 24
                            color: model.id === pipeWireManager.defaultSink ? "#44aa44" : "#2a2a2c"
                            border.color: "#ffffff22"
                            border.width: 1

                            Image {
                                anchors.centerIn: parent
                                source: "qrc:/icons/wifi-warning.svg"
                                width: 24
                                height: 24
                                color: model.id === pipeWireManager.defaultSink ? "#ffffff" : "#888888"
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Label {
                                text: model.description || model.name || "Unknown Device"
                                font.pixelSize: 18
                                font.weight: Font.Medium
                                color: "#ffffff"
                            }
                            Label {
                                text: "ID: " + model.id + (model.isDefault ? " • Default" : "")
                                font.pixelSize: 13
                                color: "#888888"
                            }
                        }

                        Item { Layout.fillWidth: true }

                        // Volume slider for this sink
                        Column {
                            width: 120
                            spacing: 4
                            anchors.verticalCenter: parent.verticalCenter

                            Row {
                                spacing: 8
                                Image {
                                    source: model.muted ? "qrc:/icons/close.svg" : "qrc:/icons/wifi-warning.svg"
                                    width: 16
                                    height: 16
                                    color: "#ffffff"
                                }
                                Slider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 1
                                    value: model.volume !== undefined ? model.volume : 1.0
                                    stepSize: 0.05
                                    onValueChanged: {
                                        if (!pressed) pipeWireManager.setSinkVolume(model.id, value)
                                    }
                                }
                            }

                            CheckBox {
                                text: "Mute"
                                checked: model.muted !== undefined ? model.muted : false
                                onToggled: pipeWireManager.setSinkMuted(model.id, checked)
                            }
                        }

                        // Set as default button
                        Button {
                            visible: model.id !== pipeWireManager.defaultSink
                            text: "Set Default"
                            onClicked: pipeWireManager.setDefaultSink(model.id)
                        }
                    }
                }
            }

            Label {
                visible: pipeWireManager.audioSinks.length === 0
                text: pipeWireManager.available ? "No output devices found" : "PipeWire not available"
                font.pixelSize: 16
                color: "#aaaaaa"
            }
        }

        // Audio Input Devices (Sources)
        SettingsSection {
            title: "Input Devices"
            spacing: 16

            ListView {
                Layout.fillWidth: true
                height: 200
                model: pipeWireManager.audioSources
                spacing: 8
                delegate: ItemDelegate {
                    width: parent.width
                    height: 72
                    contentItem: Row {
                        anchors.fill: parent
                        spacing: 16

                        Rectangle {
                            width: 48
                            height: 48
                            radius: 24
                            color: model.id === pipeWireManager.defaultSource ? "#44aa44" : "#2a2a2c"
                            border.color: "#ffffff22"
                            border.width: 1

                            Image {
                                anchors.centerIn: parent
                                source: "qrc:/icons/wifi-warning.svg"
                                width: 24
                                height: 24
                                color: model.id === pipeWireManager.defaultSource ? "#ffffff" : "#888888"
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Label {
                                text: model.description || model.name || "Unknown Device"
                                font.pixelSize: 18
                                font.weight: Font.Medium
                                color: "#ffffff"
                            }
                            Label {
                                text: "ID: " + model.id + (model.isDefault ? " • Default" : "")
                                font.pixelSize: 13
                                color: "#888888"
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Column {
                            width: 120
                            spacing: 4
                            anchors.verticalCenter: parent.verticalCenter

                            Row {
                                spacing: 8
                                Image {
                                    source: model.muted ? "qrc:/icons/close.svg" : "qrc:/icons/wifi-warning.svg"
                                    width: 16
                                    height: 16
                                    color: "#ffffff"
                                }
                                Slider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 1
                                    value: model.volume !== undefined ? model.volume : 1.0
                                    stepSize: 0.05
                                    onValueChanged: {
                                        if (!pressed) pipeWireManager.setSourceVolume(model.id, value)
                                    }
                                }
                            }

                            CheckBox {
                                text: "Mute"
                                checked: model.muted !== undefined ? model.muted : false
                                onToggled: pipeWireManager.setSourceMuted(model.id, checked)
                            }
                        }

                        Button {
                            visible: model.id !== pipeWireManager.defaultSource
                            text: "Set Default"
                            onClicked: pipeWireManager.setDefaultSource(model.id)
                        }
                    }
                }
            }

            Label {
                visible: pipeWireManager.audioSources.length === 0
                text: pipeWireManager.available ? "No input devices found" : "PipeWire not available"
                font.pixelSize: 16
                color: "#aaaaaa"
            }
        }
    }
}