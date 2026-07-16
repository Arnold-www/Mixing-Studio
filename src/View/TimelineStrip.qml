import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MixingStudio

// Seek + Song Loop — single RowLayout so I/O fields never overlay the groove.
Item {
    id: root
    implicitHeight: mixerViewModel.loopEnabled ? 56 : 40
    clip: true

    function clamp01(v) {
        return Math.max(0, Math.min(1, v))
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 16

        Label {
            text: "Seek"
            color: "#c5cede"
            font.pixelSize: 12
            font.bold: true
            Layout.preferredWidth: 40
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            id: timeline
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height
            Layout.minimumWidth: 120
            Layout.rightMargin: 8
            // Allow I/O handles to paint fully; do not clip the Out marker.
            clip: false

            readonly property real playP: mixerViewModel.playbackProgress
            readonly property real inP: mixerViewModel.loopStartProgress
            readonly property real outP: mixerViewModel.loopEndProgress
            // Keep groove inset so O handle (12px) fits when outP === 1.
            readonly property real handlePad: 8

            Rectangle {
                id: groove
                anchors.left: parent.left
                anchors.leftMargin: timeline.handlePad
                anchors.right: parent.right
                anchors.rightMargin: timeline.handlePad
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: mixerViewModel.loopEnabled ? -6 : 0
                height: 12
                radius: 6
                color: "#0c1016"
                border.color: "#2a3140"
                clip: true

                Rectangle {
                    visible: mixerViewModel.loopEnabled
                    x: groove.width * timeline.inP
                    width: Math.max(2, groove.width * (timeline.outP - timeline.inP))
                    height: parent.height
                    color: "#3d9b8444"
                }

                Rectangle {
                    width: Math.max(0, groove.width * timeline.playP)
                    height: parent.height
                    radius: 6
                    color: "#3d9b8499"
                }
            }

            Rectangle {
                x: groove.x + Math.max(0, Math.min(Math.max(0, groove.width - 3),
                                                    groove.width * timeline.playP - 1.5))
                anchors.verticalCenter: groove.verticalCenter
                width: 3
                height: 24
                radius: 1
                color: "#ffffff"
                z: 3
            }

            Item {
                anchors.left: groove.left
                anchors.right: groove.right
                anchors.top: groove.bottom
                height: 20
                visible: mixerViewModel.loopEnabled
                z: 4

                Rectangle {
                    x: Math.max(0, parent.width * timeline.inP - 6)
                    width: 12
                    height: 18
                    radius: 2
                    color: "#3d9b84"
                    Text {
                        anchors.centerIn: parent
                        text: "I"
                        color: "#0a0c10"
                        font.pixelSize: 10
                        font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.SizeHorCursor
                        drag.target: parent
                        drag.axis: Drag.XAxis
                        drag.minimumX: -6
                        drag.maximumX: parent.parent.width - 6
                        onPositionChanged: {
                            if (!pressed)
                                return
                            var p = root.clamp01((parent.x + 6) / Math.max(1, parent.parent.width))
                            mixerViewModel.setLoopRangeByProgress(p, Math.max(p + 0.02, timeline.outP))
                        }
                    }
                }

                Rectangle {
                    x: Math.max(0, parent.width * timeline.outP - 6)
                    width: 12
                    height: 18
                    radius: 2
                    color: "#e07a5f"
                    Text {
                        anchors.centerIn: parent
                        text: "O"
                        color: "#0a0c10"
                        font.pixelSize: 10
                        font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.SizeHorCursor
                        drag.target: parent
                        drag.axis: Drag.XAxis
                        drag.minimumX: -6
                        drag.maximumX: parent.parent.width - 6
                        onPositionChanged: {
                            if (!pressed)
                                return
                            var p = root.clamp01((parent.x + 6) / Math.max(1, parent.parent.width))
                            mixerViewModel.setLoopRangeByProgress(Math.min(timeline.inP, p - 0.02), p)
                        }
                    }
                }
            }

            MouseArea {
                anchors.left: groove.left
                anchors.right: groove.right
                anchors.verticalCenter: groove.verticalCenter
                height: 22
                cursorShape: Qt.PointingHandCursor
                z: 2
                onPressed: function(mouse) {
                    mixerViewModel.seekToProgress(root.clamp01(mouse.x / Math.max(1, width)))
                }
                onPositionChanged: function(mouse) {
                    if (pressed)
                        mixerViewModel.seekToProgress(root.clamp01(mouse.x / Math.max(1, width)))
                }
            }
        }

        NumericValueField {
            visible: mixerViewModel.loopEnabled
            Layout.preferredWidth: 40
            Layout.maximumWidth: 40
            Layout.alignment: Qt.AlignVCenter
            from: 0.0; to: 1.0; decimals: 0; percentScale: true
            value: mixerViewModel.loopStartProgress
            onValueEdited: function(v) {
                mixerViewModel.setLoopRangeByProgress(
                    v, Math.max(v + 0.02, mixerViewModel.loopEndProgress))
            }
        }
        NumericValueField {
            visible: mixerViewModel.loopEnabled
            Layout.preferredWidth: 40
            Layout.maximumWidth: 40
            Layout.alignment: Qt.AlignVCenter
            from: 0.0; to: 1.0; decimals: 0; percentScale: true
            value: mixerViewModel.loopEndProgress
            onValueEdited: function(v) {
                mixerViewModel.setLoopRangeByProgress(
                    Math.min(mixerViewModel.loopStartProgress, v - 0.02), v)
            }
        }

        ToolIconButton {
            text: "Song Loop"
            implicitWidth: 84
            Layout.preferredWidth: 84
            Layout.alignment: Qt.AlignVCenter
            toggled: mixerViewModel.loopEnabled
            primary: mixerViewModel.loopEnabled
            onClicked: mixerViewModel.loopEnabled = !mixerViewModel.loopEnabled
        }
    }
}
