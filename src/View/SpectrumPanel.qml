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
