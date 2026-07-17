import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MixingStudio

Item {
    id: root
    readonly property color plotTextColor: "#6a7588"
    readonly property color plotBorderColor: "#2a3140"
    readonly property color waveformColor: "#5dcea8"
    readonly property color automationColor: "#e0a45a"
    readonly property color playheadColor: "#e07a5f"
    // Do NOT use anchors.fill when this is a Layout child — that escapes the
    // layout cell and paints the playhead over sibling rows (e.g. Loop).
    // Parent (or Layout) sizes this item; we clip all plot drawing.
    clip: true

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
            property var displayPoints: []

            function syncDisplayPoints() {
                var src = mixerViewModel.waveformPoints
                if (!src || src.length === 0) {
                    displayPoints = []
                    return
                }
                var alpha = mixerViewModel.playing ? 0.36 : 1.0
                var next = new Array(src.length)
                for (var i = 0; i < src.length; ++i) {
                    var prev = (displayPoints && displayPoints.length === src.length) ? displayPoints[i] : src[i]
                    next[i] = prev + (src[i] - prev) * alpha
                }
                displayPoints = next
            }

            Canvas {
                id: waveformCanvas
                anchors.fill: parent
                renderTarget: Canvas.FramebufferObject
                renderStrategy: Canvas.Threaded
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
                    ctx.fillStyle = root.plotTextColor
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

                    ctx.strokeStyle = root.plotBorderColor
                    ctx.strokeRect(left, top, plotWidth, plotHeight)

                    var points = plotArea.displayPoints
                    if (points && points.length > 1) {
                        ctx.beginPath()
                        var x0 = left
                        var y0 = midY - points[0] * plotHeight * 0.42
                        ctx.moveTo(x0, midY)
                        ctx.lineTo(x0, y0)
                        for (var i = 0; i < points.length - 1; ++i) {
                            var x1 = left + (i / (points.length - 1)) * plotWidth
                            var x2 = left + ((i + 1) / (points.length - 1)) * plotWidth
                            var y1 = midY - points[i] * plotHeight * 0.42
                            var y2 = midY - points[i + 1] * plotHeight * 0.42
                            ctx.quadraticCurveTo(x1, y1, (x1 + x2) / 2, (y1 + y2) / 2)
                        }
                        var xLast = left + plotWidth
                        var yLast = midY - points[points.length - 1] * plotHeight * 0.42
                        ctx.quadraticCurveTo(xLast, yLast, xLast, yLast)
                        ctx.lineTo(xLast, midY)
                        ctx.closePath()
                        var fill = ctx.createLinearGradient(0, top, 0, top + plotHeight)
                        fill.addColorStop(0, "rgba(61, 155, 132, 0.38)")
                        fill.addColorStop(1, "rgba(61, 155, 132, 0.05)")
                        ctx.fillStyle = fill
                        ctx.fill()

                        ctx.beginPath()
                        ctx.moveTo(x0, y0)
                        for (var j = 0; j < points.length - 1; ++j) {
                            var ax = left + (j / (points.length - 1)) * plotWidth
                            var bx = left + ((j + 1) / (points.length - 1)) * plotWidth
                            var ay = midY - points[j] * plotHeight * 0.42
                            var by = midY - points[j + 1] * plotHeight * 0.42
                            ctx.quadraticCurveTo(ax, ay, (ax + bx) / 2, (ay + by) / 2)
                        }
                        ctx.quadraticCurveTo(xLast, yLast, xLast, yLast)
                        ctx.strokeStyle = root.waveformColor
                        ctx.lineWidth = 2.4
                        ctx.lineJoin = "round"
                        ctx.lineCap = "round"
                        ctx.stroke()
                    }

                    var autoPts = mixerViewModel.automationPoints
                    if (autoPts && autoPts.length > 0) {
                        ctx.strokeStyle = root.automationColor
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
                            ctx.fillStyle = root.automationColor
                            ctx.beginPath()
                            ctx.arc(left + autoPts[q].progress * plotWidth,
                                    top + (1.0 - autoPts[q].value) * plotHeight,
                                    4, 0, Math.PI * 2)
                            ctx.fill()
                        }
                    }
                }

                Component.onCompleted: {
                    plotArea.syncDisplayPoints()
                    requestPaint()
                }
                onWidthChanged: schedulePaint()
                onHeightChanged: schedulePaint()

                Connections {
                    target: mixerViewModel
                    function onWaveformPointsChanged() {
                        plotArea.syncDisplayPoints()
                        waveformCanvas.schedulePaint()
                    }
                    function onAutomationPointsChanged() { waveformCanvas.schedulePaint() }
                    function onSelectedTrackIndexChanged() { waveformCanvas.schedulePaint() }
                    function onPlayingChanged() {
                        plotArea.syncDisplayPoints()
                        waveformCanvas.schedulePaint()
                    }
                }
            }

            Timer {
                interval: 16
                running: mixerViewModel.playing
                repeat: true
                onTriggered: {
                    plotArea.syncDisplayPoints()
                    waveformCanvas.schedulePaint()
                }
            }

            Rectangle {
                x: plotArea.leftPad + mixerViewModel.playbackProgress * plotArea.plotWidth - 1
                y: plotArea.topPad
                width: 2
                height: plotArea.plotHeight
                color: root.playheadColor
            }

            MouseArea {
                x: plotArea.leftPad
                y: plotArea.topPad
                width: plotArea.plotWidth
                height: plotArea.plotHeight
                acceptedButtons: Qt.LeftButton
                property int dragIndex: -1

                function hitIndex(mx, my) {
                    var pts = mixerViewModel.automationPoints
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
                    if (mixerViewModel.selectedTrackIndex < 0)
                        return
                    dragIndex = hitIndex(mouse.x, mouse.y)
                    if (dragIndex < 0) {
                        mixerViewModel.addAutomationPoint(
                                    Math.max(0, Math.min(1, mouse.x / Math.max(1, width))),
                                    Math.max(0, Math.min(1, 1.0 - mouse.y / Math.max(1, height))))
                        dragIndex = mixerViewModel.automationPoints.length - 1
                    }
                }
                onPositionChanged: function(mouse) {
                    if (dragIndex < 0)
                        return
                    mixerViewModel.moveAutomationPoint(
                                dragIndex,
                                Math.max(0, Math.min(1, mouse.x / Math.max(1, width))),
                                Math.max(0, Math.min(1, 1.0 - mouse.y / Math.max(1, height))))
                }
                onReleased: dragIndex = -1
            }
    }
}
