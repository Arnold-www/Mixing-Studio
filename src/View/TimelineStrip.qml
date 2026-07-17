import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MixingStudio

// Seek + Song Loop — single RowLayout so I/O fields never overlay the groove.
Item {
    id: root
    implicitHeight: root.loopEnabled ? 56 : 40
    clip: true

    property real playbackProgress: 0
    property bool loopEnabled: false
    property real loopStartProgress: 0
    property real loopEndProgress: 1

    signal seekRequested(real progress)
    signal loopRangeRequested(real startProgress, real endProgress)
    signal loopEnabledToggled()

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
            clip: false

            readonly property real playP: root.playbackProgress
            readonly property real inP: root.loopStartProgress
            readonly property real outP: root.loopEndProgress
            readonly property real handlePad: 8

            Rectangle {
                id: groove
                anchors.left: parent.left
                anchors.leftMargin: timeline.handlePad
                anchors.right: parent.right
                anchors.rightMargin: timeline.handlePad
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: root.loopEnabled ? -6 : 0
                height: 12
                radius: 6
                color: "#0c1016"
                border.color: "#2a3140"
                clip: true

                Rectangle {
                    visible: root.loopEnabled
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
                visible: root.loopEnabled
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
                            root.loopRangeRequested(p, Math.max(p + 0.02, timeline.outP))
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
                            root.loopRangeRequested(Math.min(timeline.inP, p - 0.02), p)
                        }
                    }
                }
            }

            MouseArea {
                objectName: "seekArea"
                anchors.left: groove.left
                anchors.right: groove.right
                anchors.verticalCenter: groove.verticalCenter
                height: 22
                cursorShape: Qt.PointingHandCursor
                z: 2
                onPressed: function(mouse) {
                    root.seekRequested(root.clamp01(mouse.x / Math.max(1, width)))
                }
                onPositionChanged: function(mouse) {
                    if (pressed)
                        root.seekRequested(root.clamp01(mouse.x / Math.max(1, width)))
                }
            }
        }

        NumericValueField {
            visible: root.loopEnabled
            Layout.preferredWidth: 40
            Layout.maximumWidth: 40
            Layout.alignment: Qt.AlignVCenter
            from: 0.0; to: 1.0; decimals: 0; percentScale: true
            value: root.loopStartProgress
            onValueEdited: function(v) {
                root.loopRangeRequested(v, Math.max(v + 0.02, root.loopEndProgress))
            }
        }
        NumericValueField {
            visible: root.loopEnabled
            Layout.preferredWidth: 40
            Layout.maximumWidth: 40
            Layout.alignment: Qt.AlignVCenter
            from: 0.0; to: 1.0; decimals: 0; percentScale: true
            value: root.loopEndProgress
            onValueEdited: function(v) {
                root.loopRangeRequested(Math.min(root.loopStartProgress, v - 0.02), v)
            }
        }

        ToolIconButton {
            objectName: "songLoopButton"
            text: "Song Loop"
            implicitWidth: 84
            Layout.preferredWidth: 84
            Layout.alignment: Qt.AlignVCenter
            toggled: root.loopEnabled
            primary: root.loopEnabled
            onClicked: root.loopEnabledToggled()
        }
    }
}
