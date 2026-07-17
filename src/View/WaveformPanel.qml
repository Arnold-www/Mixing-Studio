import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MixingStudio

Item {
    id: root
    // Do NOT use anchors.fill when this is a Layout child — that escapes the
    // layout cell and paints the playhead over sibling rows (e.g. Loop).
    // Parent (or Layout) sizes this item; we clip all plot drawing.
    clip: true

    property bool playing: false
    property real playbackProgress: 0
    property var waveformPoints: []
    property var automationPoints: []
    property int selectedTrackIndex: -1

    signal addAutomationRequested(real progress, real value)
    signal moveAutomationRequested(int pointIndex, real progress, real value)

    Item {
        id: plotArea
        anchors.fill: parent
        anchors.margins: 8
        clip: true

            readonly property real leftPad: 44
            readonly property real rightPad: 12
            readonly property real topPad: 10
            readonly property real bottomPad: 26
            readonly property real plotWidth: Math.max(1, width - leftPad - rightPad)
            readonly property real plotHeight: Math.max(1, height - topPad - bottomPad)
            Canvas {
                id: waveformCanvas
                anchors.fill: parent
                // Waveform is static overview — paint on data/size change only.
                // Playhead is a separate Rectangle so Seek stays smooth at ~120 Hz.
                renderTarget: Canvas.FramebufferObject
                renderStrategy: Canvas.Cooperative
                antialiasing: true
                property bool paintPending: false

                function schedulePaint() {
                    if (paintPending)
                        return
                    paintPending = true
                    Qt.callLater(function() {
                        paintPending = false
                        requestPaint()
                    })
                }

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    var left = plotArea.leftPad
                    var top = plotArea.topPad
                    var plotWidth = plotArea.plotWidth
                    var plotHeight = plotArea.plotHeight
                    var midY = top + plotHeight / 2

                    var bg = ctx.createLinearGradient(0, top, 0, top + plotHeight)
                    bg.addColorStop(0, "#121a24")
                    bg.addColorStop(1, "#0d131b")
                    ctx.fillStyle = bg
                    ctx.fillRect(left, top, plotWidth, plotHeight)

                    ctx.strokeStyle = "rgba(80, 100, 120, 0.35)"
                    ctx.lineWidth = 1
                    ctx.fillStyle = Theme.textMuted
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
                        ctx.lineTo(left + plotWidth, gridY)
                        ctx.stroke()
                        ctx.fillText(ampLabels[a].label, left - 8, gridY)
                    }

                    ctx.textAlign = "center"
                    ctx.textBaseline = "top"
                    var timeLabels = ["0:00", "0:45", "1:30", "2:15", "3:00"]
                    for (var t = 0; t < timeLabels.length; ++t) {
                        var tx = left + (plotWidth * t / (timeLabels.length - 1))
                        ctx.beginPath()
                        ctx.moveTo(tx, top)
                        ctx.lineTo(tx, top + plotHeight)
                        ctx.stroke()
                        ctx.fillText(timeLabels[t], tx, top + plotHeight + 6)
                    }

                    ctx.strokeStyle = Theme.border
                    ctx.strokeRect(left, top, plotWidth, plotHeight)

                    var points = root.waveformPoints
                    if (points && points.length > 1) {
                        ctx.beginPath()
                        var x0 = left
                        var y0 = midY - points[0] * plotHeight * 0.42
                        ctx.moveTo(x0, midY)
                        ctx.lineTo(x0, y0)
                        // Linear segments — cheaper than quadratic curves, still realtime-smooth playhead.
                        for (var i = 1; i < points.length; ++i) {
                            var x = left + (i / (points.length - 1)) * plotWidth
                            var y = midY - points[i] * plotHeight * 0.42
                            ctx.lineTo(x, y)
                        }
                        var xLast = left + plotWidth
                        ctx.lineTo(xLast, midY)
                        ctx.closePath()
                        var fill = ctx.createLinearGradient(0, top, 0, top + plotHeight)
                        fill.addColorStop(0, "rgba(61, 155, 132, 0.38)")
                        fill.addColorStop(1, "rgba(61, 155, 132, 0.05)")
                        ctx.fillStyle = fill
                        ctx.fill()

                        ctx.beginPath()
                        ctx.moveTo(x0, y0)
                        for (var j = 1; j < points.length; ++j) {
                            var sx = left + (j / (points.length - 1)) * plotWidth
                            var sy = midY - points[j] * plotHeight * 0.42
                            ctx.lineTo(sx, sy)
                        }
                        ctx.strokeStyle = Theme.waveStroke
                        ctx.lineWidth = 2.0
                        ctx.lineJoin = "round"
                        ctx.lineCap = "round"
                        ctx.stroke()
                    }

                    var autoPts = root.automationPoints
                    if (autoPts && autoPts.length > 0) {
                        ctx.strokeStyle = Theme.autoLine
                        ctx.lineWidth = 2
                        ctx.beginPath()
                        for (var p = 0; p < autoPts.length; ++p) {
                            var apx = left + autoPts[p].progress * plotWidth
                            var apy = top + (1.0 - autoPts[p].value) * plotHeight
                            if (p === 0) ctx.moveTo(apx, apy)
                            else ctx.lineTo(apx, apy)
                        }
                        ctx.stroke()
                        for (var q = 0; q < autoPts.length; ++q) {
                            ctx.fillStyle = Theme.autoLine
                            ctx.beginPath()
                            ctx.arc(left + autoPts[q].progress * plotWidth,
                                    top + (1.0 - autoPts[q].value) * plotHeight,
                                    4, 0, Math.PI * 2)
                            ctx.fill()
                        }
                    }
                }

                Component.onCompleted: requestPaint()
                onWidthChanged: schedulePaint()
                onHeightChanged: schedulePaint()
            }

            Connections {
                target: root
                function onWaveformPointsChanged() { waveformCanvas.schedulePaint() }
                function onAutomationPointsChanged() { waveformCanvas.schedulePaint() }
                function onSelectedTrackIndexChanged() { waveformCanvas.schedulePaint() }
            }

            Rectangle {
                x: plotArea.leftPad + root.playbackProgress * plotArea.plotWidth - 1
                y: plotArea.topPad
                width: 2
                height: plotArea.plotHeight
                color: Theme.playhead
            }

            MouseArea {
                x: plotArea.leftPad
                y: plotArea.topPad
                width: plotArea.plotWidth
                height: plotArea.plotHeight
                acceptedButtons: Qt.LeftButton
                property int dragIndex: -1

                function hitIndex(mx, my) {
                    var pts = root.automationPoints
                    for (var i = 0; i < pts.length; ++i) {
                        var ax = pts[i].progress * width
                        var ay = (1.0 - pts[i].value) * height
                        var dx = mx - ax
                        var dy = my - ay
                        if ((dx * dx + dy * dy) <= 64)
                            return i
                    }
                    return -1
                }

                onPressed: function(mouse) {
                    if (root.selectedTrackIndex < 0)
                        return
                    dragIndex = hitIndex(mouse.x, mouse.y)
                    if (dragIndex < 0) {
                        root.addAutomationRequested(
                                    Math.max(0, Math.min(1, mouse.x / Math.max(1, width))),
                                    Math.max(0, Math.min(1, 1.0 - mouse.y / Math.max(1, height))))
                        dragIndex = root.automationPoints.length - 1
                    }
                }
                onPositionChanged: function(mouse) {
                    if (dragIndex < 0)
                        return
                    root.moveAutomationRequested(
                                dragIndex,
                                Math.max(0, Math.min(1, mouse.x / Math.max(1, width))),
                                Math.max(0, Math.min(1, 1.0 - mouse.y / Math.max(1, height))))
                }
                onReleased: dragIndex = -1
            }
    }
}
