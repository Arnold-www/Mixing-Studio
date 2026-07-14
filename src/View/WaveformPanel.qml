import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    radius: 8
    color: "#ffffff"
    border.color: "#d6dde7"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 4

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
