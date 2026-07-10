import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1100
    height: 720
    visible: true
    title: "Mixing Studio"
    color: "#f7f8fa"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: "Mixing Studio"
                font.pixelSize: 28
                font.bold: true
                color: "#18202a"
                Layout.fillWidth: true
            }

            Label {
                text: mixerViewModel.statusMessage
                color: "#4b5563"
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignRight
                Layout.preferredWidth: 420
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 112
            radius: 8
            color: "#ffffff"
            border.color: "#d8dee8"

            GridLayout {
                anchors.fill: parent
                anchors.margins: 14
                columns: 5
                columnSpacing: 12
                rowSpacing: 10

                Button {
                    text: "Import Mock Track"
                    Layout.preferredWidth: 150
                    onClicked: mixerViewModel.importMockTrack()
                }

                Button {
                    text: mixerViewModel.playing ? "Pause" : "Play"
                    Layout.preferredWidth: 96
                    onClicked: mixerViewModel.playing ? mixerViewModel.pause() : mixerViewModel.play()
                }

                Button {
                    text: "Stop"
                    Layout.preferredWidth: 82
                    onClicked: mixerViewModel.stop()
                }

                Label {
                    text: "Master"
                    color: "#374151"
                    horizontalAlignment: Text.AlignRight
                    Layout.preferredWidth: 70
                }

                Slider {
                    from: 0.0
                    to: 1.0
                    value: mixerViewModel.masterVolume
                    onMoved: mixerViewModel.masterVolume = value
                    Layout.fillWidth: true
                }

                Label {
                    text: mixerViewModel.playbackTimeText
                    color: "#374151"
                    font.family: "Menlo"
                    Layout.preferredWidth: 150
                }

                Slider {
                    from: 0.0
                    to: 1.0
                    value: mixerViewModel.playbackProgress
                    onMoved: mixerViewModel.seekToProgress(value)
                    Layout.columnSpan: 4
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: "#ffffff"
            border.color: "#d8dee8"

            ListView {
                id: trackList
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8
                model: mixerViewModel.tracks
                clip: true

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 104
                    radius: 8
                    color: modelData.audible ? "#f6f8fb" : "#eef1f5"
                    border.color: modelData.solo ? "#2f7d6d" : "#d7dde7"
                    opacity: modelData.audible ? 1.0 : 0.72

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Label {
                            text: modelData.name
                            font.bold: true
                            color: "#18202a"
                            elide: Text.ElideRight
                            Layout.preferredWidth: 130
                        }

                        Label {
                            text: "Volume"
                            color: "#4b5563"
                        }

                        Slider {
                            from: 0.0
                            to: 1.0
                            value: modelData.volume
                            onMoved: modelData.volume = value
                            Layout.fillWidth: true
                        }

                        Label {
                            text: modelData.volumeText
                            color: "#374151"
                            font.family: "Menlo"
                            horizontalAlignment: Text.AlignRight
                            Layout.preferredWidth: 46
                        }

                        Label {
                            text: "Pan"
                            color: "#4b5563"
                        }

                        Slider {
                            from: -1.0
                            to: 1.0
                            value: modelData.pan
                            onMoved: modelData.pan = value
                            Layout.preferredWidth: 160
                        }

                        Label {
                            text: modelData.panText
                            color: "#374151"
                            font.family: "Menlo"
                            horizontalAlignment: Text.AlignRight
                            Layout.preferredWidth: 46
                        }

                        CheckBox {
                            text: "Mute"
                            checked: modelData.muted
                            onToggled: modelData.muted = checked
                            Layout.preferredWidth: 86
                        }

                        CheckBox {
                            text: "Solo"
                            checked: modelData.solo
                            onToggled: modelData.solo = checked
                            Layout.preferredWidth: 78
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: trackList.count === 0
                    text: "Import a mock track to start building the mix."
                    color: "#6b7280"
                }
            }
        }
    }
}
