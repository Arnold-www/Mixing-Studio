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
            height: 150
            radius: 8
            color: "#ffffff"
            border.color: "#d8dee8"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8

                    Label {
                        text: "Waveform"
                        color: "#374151"
                        font.bold: true
                    }

                    Canvas {
                        id: waveformCanvas
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            ctx.strokeStyle = "#2f7d6d"
                            ctx.lineWidth = 2
                            ctx.beginPath()

                            var points = mixerViewModel.waveformPoints
                            for (var i = 0; i < points.length; ++i) {
                                var x = points.length <= 1 ? 0 : (i / (points.length - 1)) * width
                                var y = (0.5 - points[i] * 0.42) * height
                                if (i === 0)
                                    ctx.moveTo(x, y)
                                else
                                    ctx.lineTo(x, y)
                            }

                            ctx.stroke()
                            ctx.strokeStyle = "#d8dee8"
                            ctx.lineWidth = 1
                            ctx.beginPath()
                            ctx.moveTo(0, height / 2)
                            ctx.lineTo(width, height / 2)
                            ctx.stroke()
                        }

                        Connections {
                            target: mixerViewModel
                            function onWaveformPointsChanged() {
                                waveformCanvas.requestPaint()
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.preferredWidth: 320
                    Layout.fillHeight: true
                    spacing: 8

                    Label {
                        text: "Spectrum"
                        color: "#374151"
                        font.bold: true
                    }

                    Canvas {
                        id: spectrumCanvas
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)

                            var levels = mixerViewModel.spectrumLevels
                            var gap = 4
                            var barWidth = levels.length === 0 ? width : (width - gap * (levels.length - 1)) / levels.length
                            for (var i = 0; i < levels.length; ++i) {
                                var barHeight = Math.max(2, levels[i] * height)
                                var x = i * (barWidth + gap)
                                var y = height - barHeight
                                ctx.fillStyle = levels[i] > 0.72 ? "#b34d4d" : "#536f9d"
                                ctx.fillRect(x, y, barWidth, barHeight)
                            }
                        }

                        Connections {
                            target: mixerViewModel
                            function onSpectrumLevelsChanged() {
                                spectrumCanvas.requestPaint()
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: "#ffffff"
            border.color: "#d8dee8"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 14

                ColumnLayout {
                    Layout.preferredWidth: 300
                    Layout.fillHeight: true
                    spacing: 10

                    Label {
                        text: "Assets"
                        color: "#374151"
                        font.bold: true
                    }

                    TextField {
                        text: mixerViewModel.assetSearchText
                        placeholderText: "Search assets"
                        selectByMouse: true
                        onTextEdited: mixerViewModel.assetSearchText = text
                        Layout.fillWidth: true
                    }

                    ListView {
                        id: assetList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 4
                        model: mixerViewModel.filteredAssetNames
                        onCountChanged: {
                            if (currentIndex >= count)
                                currentIndex = -1
                        }

                        delegate: ItemDelegate {
                            width: ListView.view.width
                            text: modelData
                            highlighted: ListView.isCurrentItem
                            onClicked: assetList.currentIndex = index
                        }
                    }

                    Button {
                        text: "Import Selected"
                        enabled: assetList.currentIndex >= 0
                        onClicked: mixerViewModel.importAssetByName(mixerViewModel.filteredAssetNames[assetList.currentIndex])
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Recent Projects"
                        color: "#374151"
                        font.bold: true
                    }

                    ListView {
                        id: recentProjectList
                        Layout.fillWidth: true
                        Layout.preferredHeight: 104
                        clip: true
                        spacing: 4
                        model: mixerViewModel.recentProjectNames
                        onCountChanged: {
                            if (currentIndex >= count)
                                currentIndex = -1
                        }

                        delegate: ItemDelegate {
                            width: ListView.view.width
                            text: modelData
                            highlighted: ListView.isCurrentItem
                            onClicked: recentProjectList.currentIndex = index
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                            text: "Restore"
                            enabled: recentProjectList.currentIndex >= 0
                            onClicked: mixerViewModel.restoreRecentProject(mixerViewModel.recentProjectNames[recentProjectList.currentIndex])
                            Layout.fillWidth: true
                        }

                        Button {
                            text: "Save Snapshot"
                            onClicked: mixerViewModel.saveMockProject()
                            Layout.fillWidth: true
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 8
                    color: "#fafbfc"
                    border.color: "#e2e7ef"

                    ListView {
                        id: trackList
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8
                        model: mixerViewModel.tracks
                        clip: true

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 156
                            radius: 8
                            color: modelData.audible ? "#f6f8fb" : "#eef1f5"
                            border.color: modelData.solo ? "#2f7d6d" : "#d7dde7"
                            opacity: modelData.audible ? 1.0 : 0.72

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Label {
                                        text: modelData.name
                                        font.bold: true
                                        color: "#18202a"
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    Label {
                                        text: modelData.audible ? "Audible" : "Silent"
                                        color: "#374151"
                                        horizontalAlignment: Text.AlignRight
                                        Layout.preferredWidth: 72
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

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Label {
                                        text: "Volume"
                                        color: "#4b5563"
                                        Layout.preferredWidth: 62
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
                                        Layout.preferredWidth: 34
                                    }

                                    Slider {
                                        from: -1.0
                                        to: 1.0
                                        value: modelData.pan
                                        onMoved: modelData.pan = value
                                        Layout.preferredWidth: 150
                                    }

                                    Label {
                                        text: modelData.panText
                                        color: "#374151"
                                        font.family: "Menlo"
                                        horizontalAlignment: Text.AlignRight
                                        Layout.preferredWidth: 46
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Label {
                                        text: "Level"
                                        color: "#4b5563"
                                        Layout.preferredWidth: 130
                                    }

                                    ProgressBar {
                                        from: 0.0
                                        to: 1.0
                                        value: modelData.meterLevel
                                        Layout.fillWidth: true
                                    }

                                    Label {
                                        text: modelData.meterText
                                        color: "#374151"
                                        font.family: "Menlo"
                                        horizontalAlignment: Text.AlignRight
                                        Layout.preferredWidth: 46
                                    }
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
    }
}
