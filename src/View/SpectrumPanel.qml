import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MixingStudio

Item {
    id: root
    // Sized by Layout parent — do not anchors.fill outer drawer (overlaps header).
    clip: true

    property bool showCloseButton: true
    signal closeRequested()

    // Unified header — title / subtitle left, status + Close right
    Item {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 40

        Label {
            id: titleText
            text: "Spectrum"
            color: "#ffffff"
            font.bold: true
            font.pixelSize: 14
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
        }

        Label {
            id: subtitleText
            text: "faux-3D history"
            color: "#A0A0A0"
            font.pixelSize: 11
            anchors.left: titleText.right
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
        }

        ToolIconButton {
            id: closeButton
            visible: root.showCloseButton
            text: "Close"
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            z: 2
            onClicked: root.closeRequested()
        }

        Label {
            id: statusText
            text: mixerViewModel.playing ? "realtime · 60 FPS" : "idle"
            color: mixerViewModel.playing ? "#5dcea8" : "#A0A0A0"
            font.pixelSize: 11
            font.family: "Consolas"
            anchors.right: root.showCloseButton ? closeButton.left : parent.right
            anchors.rightMargin: root.showCloseButton ? 10 : 4
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Canvas {
            id: spectrumCanvas
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: header.bottom
            anchors.bottom: parent.bottom
            anchors.topMargin: 4
            clip: true
            renderTarget: Canvas.FramebufferObject
            renderStrategy: Canvas.Threaded
            antialiasing: true

            property var smoothLevels: []
            property var history: []
            readonly property int historyDepth: 16
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

            function pushLevels(levels) {
                if (!levels || levels.length === 0)
                    return
                var attack = 0.58
                var release = 0.22
                var next = new Array(levels.length)
                for (var i = 0; i < levels.length; ++i) {
                    var prev = (smoothLevels.length === levels.length) ? smoothLevels[i] : 0
                    var t = levels[i]
                    next[i] = prev + (t - prev) * ((t >= prev) ? attack : release)
                }
                smoothLevels = next
                var hist = history.slice()
                hist.unshift(next.slice())
                if (hist.length > historyDepth)
                    hist.length = historyDepth
                history = hist
            }

            function barColor(level, depthFade) {
                var hot = Math.max(0, Math.min(1, (level - 0.5) / 0.5))
                var r = Math.floor(40 + hot * 170 + (1.0 - depthFade) * 20)
                var g = Math.floor(150 - hot * 50 + (1.0 - depthFade) * 40)
                var b = Math.floor(160 - hot * 90 + depthFade * 50)
                var alpha = 0.18 + (1.0 - depthFade) * 0.75
                return "rgba(" + r + "," + g + "," + b + "," + alpha.toFixed(3) + ")"
            }

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                var bg = ctx.createLinearGradient(0, 0, 0, height)
                bg.addColorStop(0, "#101820")
                bg.addColorStop(1, "#0a1016")
                ctx.fillStyle = bg
                ctx.fillRect(0, 0, width, height)

                var left = 16
                var right = 12
                var top = 8
                var bottom = 20
                var plotW = width - left - right
                var plotH = height - top - bottom
                var depth = history.length > 0 ? history.length : 1

                ctx.strokeStyle = "rgba(100, 130, 160, 0.14)"
                ctx.lineWidth = 1
                for (var g = 0; g < 7; ++g) {
                    var gt = g / 6
                    var gy = top + plotH * (0.18 + gt * 0.74)
                    var inset = plotW * (0.20 * (1.0 - gt))
                    ctx.beginPath()
                    ctx.moveTo(left + inset, gy)
                    ctx.lineTo(left + plotW - inset, gy)
                    ctx.stroke()
                }

                for (var row = depth - 1; row >= 0; --row) {
                    var rowLevels = history.length > 0 ? history[row] : smoothLevels
                    if (!rowLevels || rowLevels.length === 0)
                        continue

                    var depthT = row / Math.max(1, historyDepth - 1)
                    var scale = 1.0 - depthT * 0.58
                    var yBase = top + plotH * (0.92 - depthT * 0.60)
                    var xInset = plotW * (0.5 - 0.5 * scale)
                    var rowW = plotW * scale
                    var maxBarH = plotH * (0.46 * scale)
                    var gap = Math.max(1, rowW * 0.01)
                    var barW = (rowW - gap * (rowLevels.length - 1)) / rowLevels.length
                    var skew = 7 * (1.0 - depthT)

                    for (var i = 0; i < rowLevels.length; ++i) {
                        var level = Math.max(0, Math.min(1, rowLevels[i]))
                        var h = Math.max(2, level * maxBarH)
                        var x = left + xInset + i * (barW + gap)
                        var y = yBase - h

                        ctx.beginPath()
                        ctx.moveTo(x, yBase)
                        ctx.lineTo(x + skew * 0.4, y)
                        ctx.lineTo(x + barW + skew * 0.4, y)
                        ctx.lineTo(x + barW, yBase)
                        ctx.closePath()
                        ctx.fillStyle = barColor(level, depthT)
                        ctx.fill()

                        if (row === 0) {
                            ctx.fillStyle = "rgba(210, 245, 235, 0.40)"
                            ctx.fillRect(x + skew * 0.4, y, barW, 2)
                        }
                    }
                }

                ctx.strokeStyle = "rgba(148, 163, 184, 0.28)"
                ctx.strokeRect(left, top, plotW, plotH)

                ctx.fillStyle = Theme.textMuted
                ctx.font = "10px sans-serif"
                ctx.textAlign = "center"
                ctx.textBaseline = "top"
                var labels = ["60", "250", "1k", "4k", "16k"]
                for (var f = 0; f < labels.length; ++f) {
                    var fx = left + plotW * (0.08 + 0.84 * f / (labels.length - 1))
                    ctx.fillText(labels[f], fx, top + plotH + 3)
                }
            }

            Component.onCompleted: {
                pushLevels(mixerViewModel.spectrumLevels)
                requestPaint()
            }
            onWidthChanged: schedulePaint()
            onHeightChanged: schedulePaint()

            Connections {
                target: mixerViewModel
                function onSpectrumLevelsChanged() {
                    spectrumCanvas.pushLevels(mixerViewModel.spectrumLevels)
                    spectrumCanvas.schedulePaint()
                }
                function onPlayingChanged() { spectrumCanvas.schedulePaint() }
            }

            Timer {
                interval: 16
                running: true
                repeat: true
                onTriggered: {
                    if (mixerViewModel.playing) {
                        spectrumCanvas.pushLevels(mixerViewModel.spectrumLevels)
                    } else if (spectrumCanvas.smoothLevels.length > 0) {
                        var faded = []
                        for (var i = 0; i < spectrumCanvas.smoothLevels.length; ++i)
                            faded.push(spectrumCanvas.smoothLevels[i] * 0.90)
                        spectrumCanvas.pushLevels(faded)
                    }
                    spectrumCanvas.schedulePaint()
                }
            }
    }
}
