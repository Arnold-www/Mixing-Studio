import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1100
    height: 720
    visible: true
    title: "Mixing Studio"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: "Mixing Studio - MVVM Skeleton"
            font.pixelSize: 26
            font.bold: true
        }

        RowLayout {
            spacing: 8

            Button {
                text: "Import Mock Track"
                onClicked: mixerViewModel.importMockTrack()
            }

            Button {
                text: mixerViewModel.playing ? "Pause" : "Play"
                onClicked: mixerViewModel.playing ? mixerViewModel.pause() : mixerViewModel.play()
            }

            Button {
                text: "Stop"
                onClicked: mixerViewModel.stop()
            }

            Label {
                text: "Master"
            }

            Slider {
                from: 0.0
                to: 1.0
                value: mixerViewModel.masterVolume
                onMoved: mixerViewModel.masterVolume = value
                Layout.preferredWidth: 180
            }
        }

        Label {
            text: mixerViewModel.statusMessage
            color: "#666666"
        }

        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                anchors.fill: parent
                spacing: 8
                model: mixerViewModel.tracks

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 96
                    radius: 8
                    color: "#f3f4f6"
                    border.color: "#d1d5db"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Label {
                            text: modelData.name
                            font.bold: true
                            Layout.preferredWidth: 120
                        }

                        Label {
                            text: "Volume"
                        }

                        Slider {
                            from: 0.0
                            to: 1.0
                            value: modelData.volume
                            onMoved: modelData.volume = value
                            Layout.fillWidth: true
                        }

                        Label {
                            text: "Pan"
                        }

                        Slider {
                            from: -1.0
                            to: 1.0
                            value: modelData.pan
                            onMoved: modelData.pan = value
                            Layout.preferredWidth: 160
                        }

                        CheckBox {
                            text: "Mute"
                            checked: modelData.muted
                            onToggled: modelData.muted = checked
                        }

                        CheckBox {
                            text: "Solo"
                            checked: modelData.solo
                            onToggled: modelData.solo = checked
                        }
                    }
                }
            }
        }
    }
}
