import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1220
    height: 780
    minimumWidth: 1040
    minimumHeight: 680
    visible: true
    title: "Mixing Studio"
    color: "#eef2f5"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 88
            radius: 8
            color: "#ffffff"
            border.color: "#d6dde7"

            GridLayout {
                anchors.fill: parent
                anchors.margins: 12
                columns: 8
                columnSpacing: 10
                rowSpacing: 8

                Label {
                    text: "Mixing Studio"
                    font.pixelSize: 24
                    font.bold: true
                    color: "#18202a"
                    Layout.preferredWidth: 190
                }

                Button {
                    text: "Import"
                    Layout.preferredWidth: 86
                    onClicked: mixerViewModel.importMockTrack()
                }

                Button {
                    text: mixerViewModel.playing ? "Pause" : "Play"
                    Layout.preferredWidth: 82
                    onClicked: mixerViewModel.playing ? mixerViewModel.pause() : mixerViewModel.play()
                }

                Button {
                    text: "Stop"
                    Layout.preferredWidth: 74
                    onClicked: mixerViewModel.stop()
                }

                Label {
                    text: mixerViewModel.playbackTimeText
                    color: "#374151"
                    font.family: "Menlo"
                    horizontalAlignment: Text.AlignRight
                    Layout.preferredWidth: 132
                }

                Slider {
                    from: 0.0
                    to: 1.0
                    value: mixerViewModel.playbackProgress
                    onMoved: mixerViewModel.seekToProgress(value)
                    Layout.fillWidth: true
                }

                Label {
                    text: "Master"
                    color: "#374151"
                    horizontalAlignment: Text.AlignRight
                    Layout.preferredWidth: 58
                }

                Slider {
                    from: 0.0
                    to: 1.0
                    value: mixerViewModel.masterVolume
                    onMoved: mixerViewModel.masterVolume = value
                    Layout.preferredWidth: 180
                }

                Label {
                    text: mixerViewModel.statusMessage
                    color: "#4b5563"
                    elide: Text.ElideRight
                    Layout.columnSpan: 8
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 214
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 8
                color: "#ffffff"
                border.color: "#d6dde7"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "Waveform"
                            color: "#253241"
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        Label {
                            text: "time"
                            color: "#6b7280"
                        }
                    }

                    Canvas {
                        id: waveformCanvas
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)

                            var left = 42
                            var right = 10
                            var top = 8
                            var bottom = 24
                            var plotWidth = width - left - right
                            var plotHeight = height - top - bottom
                            var midY = top + plotHeight / 2

                            ctx.strokeStyle = "#e2e8f0"
                            ctx.lineWidth = 1
                            ctx.fillStyle = "#6b7280"
                            ctx.font = "10px sans-serif"
                            ctx.textAlign = "right"
                            ctx.textBaseline = "middle"

                            var ampLabels = [
                                { label: "+1", value: -1 },
                                { label: "0", value: 0 },
                                { label: "-1", value: 1 }
                            ]

                            for (var a = 0; a < ampLabels.length; ++a) {
                                var gridY = midY + ampLabels[a].value * plotHeight * 0.42
                                ctx.beginPath()
                                ctx.moveTo(left, gridY)
                                ctx.lineTo(width - right, gridY)
                                ctx.stroke()
                                ctx.fillText(ampLabels[a].label, left - 8, gridY)
                            }

                            ctx.textAlign = "center"
                            ctx.textBaseline = "top"
                            var timeLabels = ["0:00", "0:45", "1:30", "2:15", "3:00"]
                            for (var t = 0; t < timeLabels.length; ++t) {
                                var x = left + (plotWidth * t / (timeLabels.length - 1))
                                ctx.beginPath()
                                ctx.moveTo(x, top)
                                ctx.lineTo(x, top + plotHeight)
                                ctx.stroke()
                                ctx.fillText(timeLabels[t], x, top + plotHeight + 6)
                            }

                            ctx.strokeStyle = "#94a3b8"
                            ctx.strokeRect(left, top, plotWidth, plotHeight)

                            var points = mixerViewModel.waveformPoints
                            ctx.strokeStyle = "#2f7d6d"
                            ctx.lineWidth = 2
                            ctx.beginPath()
                            for (var i = 0; i < points.length; ++i) {
                                var px = left + (points.length <= 1 ? 0 : (i / (points.length - 1)) * plotWidth)
                                var py = midY - points[i] * plotHeight * 0.42
                                if (i === 0)
                                    ctx.moveTo(px, py)
                                else
                                    ctx.lineTo(px, py)
                            }
                            ctx.stroke()

                            var playX = left + mixerViewModel.playbackProgress * plotWidth
                            ctx.strokeStyle = "#b34d4d"
                            ctx.lineWidth = 2
                            ctx.beginPath()
                            ctx.moveTo(playX, top)
                            ctx.lineTo(playX, top + plotHeight)
                            ctx.stroke()
                        }

                        Component.onCompleted: requestPaint()

                        Connections {
                            target: mixerViewModel
                            function onWaveformPointsChanged() {
                                waveformCanvas.requestPaint()
                            }
                            function onPlaybackPositionChanged() {
                                waveformCanvas.requestPaint()
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
                border.color: "#d6dde7"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "Spectrum"
                            color: "#253241"
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        Label {
                            text: "frequency / level"
                            color: "#6b7280"
                        }
                    }

                    Canvas {
                        id: spectrumCanvas
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)

                            var left = 42
                            var right = 10
                            var top = 8
                            var bottom = 24
                            var plotWidth = width - left - right
                            var plotHeight = height - top - bottom

                            ctx.strokeStyle = "#e2e8f0"
                            ctx.lineWidth = 1
                            ctx.fillStyle = "#6b7280"
                            ctx.font = "10px sans-serif"
                            ctx.textAlign = "right"
                            ctx.textBaseline = "middle"

                            var dbLabels = [
                                { label: "0", value: 1.0 },
                                { label: "-12", value: 0.75 },
                                { label: "-24", value: 0.5 },
                                { label: "-48 dB", value: 0.0 }
                            ]

                            for (var d = 0; d < dbLabels.length; ++d) {
                                var y = top + plotHeight * (1.0 - dbLabels[d].value)
                                ctx.beginPath()
                                ctx.moveTo(left, y)
                                ctx.lineTo(width - right, y)
                                ctx.stroke()
                                ctx.fillText(dbLabels[d].label, left - 8, y)
                            }

                            ctx.textAlign = "center"
                            ctx.textBaseline = "top"
                            var freqLabels = ["60", "250", "1k", "4k", "12k"]
                            for (var f = 0; f < freqLabels.length; ++f) {
                                var x = left + (plotWidth * f / (freqLabels.length - 1))
                                ctx.beginPath()
                                ctx.moveTo(x, top)
                                ctx.lineTo(x, top + plotHeight)
                                ctx.stroke()
                                ctx.fillText(freqLabels[f], x, top + plotHeight + 6)
                            }

                            ctx.strokeStyle = "#94a3b8"
                            ctx.strokeRect(left, top, plotWidth, plotHeight)

                            var levels = mixerViewModel.spectrumLevels
                            var gap = 4
                            var barWidth = levels.length === 0 ? plotWidth : (plotWidth - gap * (levels.length - 1)) / levels.length
                            for (var i = 0; i < levels.length; ++i) {
                                var barHeight = Math.max(2, levels[i] * plotHeight)
                                var bx = left + i * (barWidth + gap)
                                var by = top + plotHeight - barHeight
                                ctx.fillStyle = levels[i] > 0.72 ? "#b34d4d" : "#536f9d"
                                ctx.fillRect(bx, by, barWidth, barHeight)
                            }
                        }

                        Component.onCompleted: requestPaint()

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

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 8
                color: "#ffffff"
                border.color: "#d6dde7"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "Mixer"
                            color: "#253241"
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        Label {
                            text: mixerViewModel.anySolo ? "Solo active" : "All tracks"
                            color: mixerViewModel.anySolo ? "#b34d4d" : "#6b7280"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 8
                        color: "#f8fafc"
                        border.color: "#e2e8f0"

                        ListView {
                            id: trackStripList
                            anchors.fill: parent
                            anchors.margins: 10
                            orientation: ListView.Horizontal
                            spacing: 10
                            clip: true
                            model: mixerViewModel.tracks

                            delegate: Rectangle {
                                width: 178
                                height: trackStripList.height
                                radius: 8
                                color: modelData.audible ? "#ffffff" : "#eef1f5"
                                border.color: modelData.solo ? "#2f7d6d" : "#d6dde7"
                                opacity: modelData.audible ? 1.0 : 0.74

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 8

                                    Label {
                                        text: modelData.name
                                        color: "#18202a"
                                        font.bold: true
                                        elide: Text.ElideRight
                                        horizontalAlignment: Text.AlignHCenter
                                        Layout.fillWidth: true
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        spacing: 10

                                        Rectangle {
                                            Layout.preferredWidth: 24
                                            Layout.fillHeight: true
                                            radius: 4
                                            color: "#e5eaf1"

                                            Rectangle {
                                                width: parent.width
                                                height: Math.max(2, parent.height * modelData.meterLevel)
                                                anchors.bottom: parent.bottom
                                                radius: 4
                                                color: modelData.meterLevel > 0.76 ? "#b34d4d" : "#2f7d6d"
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            spacing: 8

                                            Slider {
                                                orientation: Qt.Vertical
                                                from: 0.0
                                                to: 1.0
                                                value: modelData.volume
                                                onMoved: modelData.volume = value
                                                Layout.fillHeight: true
                                                Layout.alignment: Qt.AlignHCenter
                                            }

                                            Label {
                                                text: modelData.volumeText
                                                color: "#374151"
                                                font.family: "Menlo"
                                                horizontalAlignment: Text.AlignHCenter
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 6

                                        Label {
                                            text: "Pan"
                                            color: "#4b5563"
                                            Layout.preferredWidth: 30
                                        }

                                        Slider {
                                            from: -1.0
                                            to: 1.0
                                            value: modelData.pan
                                            onMoved: modelData.pan = value
                                            Layout.fillWidth: true
                                        }

                                        Label {
                                            text: modelData.panText
                                            color: "#374151"
                                            font.family: "Menlo"
                                            horizontalAlignment: Text.AlignRight
                                            Layout.preferredWidth: 36
                                        }
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 6

                                        CheckBox {
                                            text: "M"
                                            checked: modelData.muted
                                            onToggled: modelData.muted = checked
                                            Layout.fillWidth: true
                                        }

                                        CheckBox {
                                            text: "S"
                                            checked: modelData.solo
                                            onToggled: modelData.solo = checked
                                            Layout.fillWidth: true
                                        }
                                    }

                                    Label {
                                        text: modelData.meterText
                                        color: "#4b5563"
                                        font.family: "Menlo"
                                        horizontalAlignment: Text.AlignHCenter
                                        Layout.fillWidth: true
                                    }
                                }
                            }

                            Label {
                                anchors.centerIn: parent
                                visible: trackStripList.count === 0
                                text: "Import or select an asset to create mixer channels."
                                color: "#6b7280"
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 260
                Layout.fillHeight: true
                radius: 8
                color: "#ffffff"
                border.color: "#d6dde7"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Label {
                        text: "Library"
                        color: "#253241"
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
                        Layout.preferredHeight: 150
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
                        text: "Projects"
                        color: "#253241"
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

                    Button {
                        text: "Restore Selected"
                        enabled: recentProjectList.currentIndex >= 0
                        onClicked: mixerViewModel.restoreRecentProject(mixerViewModel.recentProjectNames[recentProjectList.currentIndex])
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Save Snapshot"
                        onClicked: mixerViewModel.saveMockProject()
                        Layout.fillWidth: true
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }
}
