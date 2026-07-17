import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MixingStudio

Rectangle {
    id: mixer
    color: "#12151c"
    border.color: "#2a3140"
    radius: 6

    property var tracks
    property int selectedTrackIndex: -1
    property bool anySolo: false

    signal trackSelected(int index)
    signal clearAutomationRequested()
    signal deleteTrackRequested()

    Material.theme: Material.Dark
    Material.accent: Material.Teal
    Material.background: "#12151c"
    Material.foreground: "#ffffff"

    readonly property int mixColW: 200
    readonly property int faderCount: 12
    readonly property real faderGap: 10

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: "Tracks"
                color: "#ffffff"
                font.bold: true
                font.pixelSize: 14
            }
            Label {
                text: mixer.tracks && mixer.tracks.length
                      ? (mixer.tracks.length + " channel" + (mixer.tracks.length > 1 ? "s" : ""))
                      : "no channels"
                color: "#c5cede"
                font.pixelSize: 12
                Layout.fillWidth: true
            }
            ToolIconButton {
                text: "Clear Auto"
                enabled: mixer.selectedTrackIndex >= 0
                onClicked: mixer.clearAutomationRequested()
            }
            ToolIconButton {
                text: "Delete"
                enabled: mixer.selectedTrackIndex >= 0
                onClicked: mixer.deleteTrackRequested()
            }
            Label {
                text: mixer.anySolo ? "Solo on" : ""
                color: "#ff8a8a"
                font.pixelSize: 12
            }
        }

        ListView {
            id: trackList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 10
            orientation: ListView.Vertical
            model: mixer.tracks
            currentIndex: mixer.selectedTrackIndex
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            delegate: Rectangle {
                id: rowRoot
                width: trackList.width
                radius: 6
                readonly property var trackVm: modelData
                readonly property bool hasTrack: trackVm !== null && trackVm !== undefined
                readonly property bool selected: index === mixer.selectedTrackIndex
                readonly property bool loopOpen: hasTrack && trackVm.loopEnabled
                property bool fxOpen: false
                property bool hovered: rowHover.containsMouse
                visible: hasTrack
                height: fxOpen ? 318 : (loopOpen ? 158 : 96)

                color: {
                    if (selected)
                        return "#1e3a34"
                    if (hovered)
                        return "#252b38"
                    return "#181c26"
                }
                border.color: selected ? "#5dcea8" : (hovered ? "#6a7a94" : "#2a3140")
                border.width: selected ? 2 : 1
                opacity: hasTrack && trackVm.audible ? 1.0 : 0.72

                Behavior on height {
                    NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
                }

                MouseArea {
                    id: rowHover
                    anchors.fill: parent
                    z: -1
                    hoverEnabled: true
                    enabled: rowRoot.hasTrack
                    onClicked: mixer.trackSelected(index)
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10
                    enabled: rowRoot.hasTrack

                    // Left controls | Vol / Pan / Speed (evenly spaced) | meter
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        ColumnLayout {
                            Layout.preferredWidth: 228
                            Layout.maximumWidth: 228
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 6

                            Label {
                                text: rowRoot.hasTrack ? trackVm.name : ""
                                color: "#ffffff"
                                font.bold: true
                                font.pixelSize: 13
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                spacing: 8
                                ToolIconButton {
                                    text: "M"
                                    implicitWidth: 36
                                    toggled: rowRoot.hasTrack ? trackVm.muted : false
                                    onClicked: if (rowRoot.hasTrack) trackVm.muted = !trackVm.muted
                                }
                                ToolIconButton {
                                    text: "S"
                                    implicitWidth: 36
                                    toggled: rowRoot.hasTrack ? trackVm.solo : false
                                    onClicked: if (rowRoot.hasTrack) trackVm.solo = !trackVm.solo
                                }
                                ToolIconButton {
                                    text: "FX"
                                    implicitWidth: 44
                                    toggled: rowRoot.fxOpen
                                    onClicked: {
                                        if (!rowRoot.hasTrack)
                                            return
                                        rowRoot.fxOpen = !rowRoot.fxOpen
                                        trackVm.fxBypass = !rowRoot.fxOpen
                                        mixer.trackSelected(index)
                                    }
                                }
                                ClipLoopToggle {
                                    Layout.preferredWidth: 96
                                    Layout.preferredHeight: 30
                                    loopEnabled: rowRoot.hasTrack ? trackVm.loopEnabled : false
                                    onToggled: {
                                        if (!rowRoot.hasTrack)
                                            return
                                        trackVm.loopEnabled = !trackVm.loopEnabled
                                        mixer.trackSelected(index)
                                    }
                                }
                            }
                        }

                        Item { Layout.fillWidth: true; Layout.minimumWidth: 12 }

                        // Vol group — longer slider for finer control
                        RowLayout {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: 256
                            Layout.maximumWidth: 280
                            spacing: 6
                            Label {
                                text: "Vol"
                                color: "#ffffff"
                                font.pixelSize: 12
                                Layout.preferredWidth: 28
                                Layout.alignment: Qt.AlignVCenter
                            }
                            Slider {
                                id: volSlider
                                from: 0.0; to: 1.0
                                onMoved: if (rowRoot.hasTrack) trackVm.volume = value
                                Layout.fillWidth: true
                                Layout.preferredWidth: 170
                                Layout.maximumWidth: 200
                                Layout.alignment: Qt.AlignVCenter
                                Binding {
                                    target: volSlider
                                    property: "value"
                                    value: rowRoot.hasTrack ? trackVm.volume : 0
                                    when: !volSlider.pressed
                                }
                            }
                            NumericValueField {
                                Layout.preferredWidth: 40
                                Layout.maximumWidth: 40
                                Layout.alignment: Qt.AlignVCenter
                                from: 0.0; to: 1.0; decimals: 0; percentScale: true
                                value: rowRoot.hasTrack ? trackVm.volume : 0
                                onValueEdited: function(v) {
                                    if (rowRoot.hasTrack) trackVm.volume = v
                                }
                            }
                            Label {
                                text: "%"
                                color: "#c5cede"
                                font.pixelSize: 11
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }

                        Item { Layout.fillWidth: true; Layout.minimumWidth: 12 }

                        RowLayout {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: 248
                            Layout.maximumWidth: 270
                            spacing: 6
                            Label {
                                text: "Pan"
                                color: "#ffffff"
                                font.pixelSize: 12
                                Layout.preferredWidth: 28
                                Layout.alignment: Qt.AlignVCenter
                            }
                            Slider {
                                id: panSlider
                                from: -1.0; to: 1.0
                                onMoved: if (rowRoot.hasTrack) trackVm.pan = value
                                Layout.fillWidth: true
                                Layout.preferredWidth: 170
                                Layout.maximumWidth: 200
                                Layout.alignment: Qt.AlignVCenter
                                Binding {
                                    target: panSlider
                                    property: "value"
                                    value: rowRoot.hasTrack ? trackVm.pan : 0
                                    when: !panSlider.pressed
                                }
                            }
                            NumericValueField {
                                Layout.preferredWidth: 40
                                Layout.maximumWidth: 40
                                Layout.alignment: Qt.AlignVCenter
                                from: -1.0; to: 1.0; decimals: 0; percentScale: true
                                value: rowRoot.hasTrack ? trackVm.pan : 0
                                onValueEdited: function(v) {
                                    if (rowRoot.hasTrack) trackVm.pan = v
                                }
                            }
                        }

                        Item { Layout.fillWidth: true; Layout.minimumWidth: 12 }

                        RowLayout {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: 268
                            Layout.maximumWidth: 290
                            spacing: 6
                            Label {
                                text: "Speed"
                                color: "#ffffff"
                                font.pixelSize: 12
                                Layout.preferredWidth: 40
                                Layout.alignment: Qt.AlignVCenter
                            }
                            Slider {
                                id: speedSlider
                                from: 0.5; to: 2.0
                                onMoved: if (rowRoot.hasTrack) trackVm.playbackRate = value
                                Layout.fillWidth: true
                                Layout.preferredWidth: 170
                                Layout.maximumWidth: 200
                                Layout.alignment: Qt.AlignVCenter
                                Binding {
                                    target: speedSlider
                                    property: "value"
                                    value: rowRoot.hasTrack ? trackVm.playbackRate : 1.0
                                    when: !speedSlider.pressed
                                }
                            }
                            NumericValueField {
                                Layout.preferredWidth: 44
                                Layout.maximumWidth: 44
                                Layout.alignment: Qt.AlignVCenter
                                from: 0.5; to: 2.0; decimals: 2; percentScale: false
                                value: rowRoot.hasTrack ? trackVm.playbackRate : 1.0
                                onValueEdited: function(v) {
                                    if (rowRoot.hasTrack) trackVm.playbackRate = v
                                }
                            }
                            Label {
                                text: "x"
                                color: "#c5cede"
                                font.pixelSize: 11
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }

                        Item { Layout.fillWidth: true; Layout.minimumWidth: 12 }

                        RowLayout {
                            Layout.preferredWidth: 168
                            Layout.maximumWidth: 168
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 8
                            Label {
                                text: rowRoot.hasTrack ? trackVm.meterText : "0%"
                                color: "#ffffff"
                                font.family: "Consolas"
                                font.pixelSize: 12
                                Layout.preferredWidth: 36
                                Layout.alignment: Qt.AlignVCenter
                                horizontalAlignment: Text.AlignRight
                            }
                            LevelMeterBar {
                                Layout.preferredWidth: 120
                                Layout.preferredHeight: 14
                                Layout.alignment: Qt.AlignVCenter
                                currentLevel: rowRoot.hasTrack ? trackVm.meterLevel : 0
                                threshold: 0.85
                            }
                        }
                    }

                    // Per-track loop + editable I / O percent fields
                    Item {
                        id: trackLoopBar
                        visible: rowRoot.loopOpen
                        Layout.fillWidth: true
                        Layout.preferredHeight: 44
                        clip: true

                        function clamp01(v) {
                            return Math.max(0, Math.min(1, v))
                        }

                        readonly property real inP: rowRoot.hasTrack ? trackVm.loopStartProgress : 0
                        readonly property real outP: rowRoot.hasTrack ? trackVm.loopEndProgress : 1

                        RowLayout {
                            anchors.fill: parent
                            spacing: 8

                            Label {
                                text: "Loop"
                                color: "#5dcea8"
                                font.pixelSize: 11
                                font.bold: true
                                Layout.preferredWidth: 36
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Label {
                                text: "I"
                                color: "#5dcea8"
                                font.pixelSize: 11
                                font.bold: true
                                Layout.alignment: Qt.AlignVCenter
                            }
                            NumericValueField {
                                Layout.preferredWidth: 40
                                Layout.alignment: Qt.AlignVCenter
                                from: 0.0; to: 1.0; decimals: 0; percentScale: true
                                value: trackLoopBar.inP
                                onValueEdited: function(v) {
                                    if (!rowRoot.hasTrack)
                                        return
                                    trackVm.setLoopRangeByProgress(v, Math.max(v + 0.02, trackLoopBar.outP))
                                }
                            }
                            Label {
                                text: "%"
                                color: "#c5cede"
                                font.pixelSize: 11
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Item {
                                id: loopTimeline
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true

                                Rectangle {
                                    id: loopGroove
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    height: 12
                                    radius: 6
                                    color: "#0c1016"
                                    border.color: "#2a3140"
                                    clip: true

                                    Rectangle {
                                        x: loopGroove.width * trackLoopBar.inP
                                        width: Math.max(4, loopGroove.width * (trackLoopBar.outP - trackLoopBar.inP))
                                        height: parent.height
                                        radius: 6
                                        color: "#2f8f7866"
                                        border.color: "#5dcea8"
                                        border.width: 1
                                    }
                                }

                                Rectangle {
                                    id: inHandle
                                    x: Math.max(0, Math.min(loopTimeline.width - width,
                                                            loopTimeline.width * trackLoopBar.inP - width / 2))
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 14
                                    height: 28
                                    radius: 3
                                    color: "#5dcea8"
                                    border.color: "#ffffff"
                                    z: 4
                                    Text {
                                        anchors.centerIn: parent
                                        text: "I"
                                        color: "#0a0c10"
                                        font.bold: true
                                        font.pixelSize: 10
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.SizeHorCursor
                                        preventStealing: true
                                        onPositionChanged: function(mouse) {
                                            if (!pressed || !rowRoot.hasTrack)
                                                return
                                            var gx = mapToItem(loopTimeline, mouse.x, 0).x
                                            var p = trackLoopBar.clamp01(gx / Math.max(1, loopTimeline.width))
                                            trackVm.setLoopRangeByProgress(p, Math.max(p + 0.02, trackLoopBar.outP))
                                        }
                                    }
                                }

                                Rectangle {
                                    id: outHandle
                                    x: Math.max(0, Math.min(loopTimeline.width - width,
                                                            loopTimeline.width * trackLoopBar.outP - width / 2))
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 14
                                    height: 28
                                    radius: 3
                                    color: "#e0a45a"
                                    border.color: "#ffffff"
                                    z: 4
                                    Text {
                                        anchors.centerIn: parent
                                        text: "O"
                                        color: "#0a0c10"
                                        font.bold: true
                                        font.pixelSize: 10
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.SizeHorCursor
                                        preventStealing: true
                                        onPositionChanged: function(mouse) {
                                            if (!pressed || !rowRoot.hasTrack)
                                                return
                                            var gx = mapToItem(loopTimeline, mouse.x, 0).x
                                            var p = trackLoopBar.clamp01(gx / Math.max(1, loopTimeline.width))
                                            trackVm.setLoopRangeByProgress(Math.min(trackLoopBar.inP, p - 0.02), p)
                                        }
                                    }
                                }
                            }

                            Label {
                                text: "O"
                                color: "#e0a45a"
                                font.pixelSize: 11
                                font.bold: true
                                Layout.alignment: Qt.AlignVCenter
                            }
                            NumericValueField {
                                Layout.preferredWidth: 40
                                Layout.alignment: Qt.AlignVCenter
                                from: 0.0; to: 1.0; decimals: 0; percentScale: true
                                value: trackLoopBar.outP
                                onValueEdited: function(v) {
                                    if (!rowRoot.hasTrack)
                                        return
                                    trackVm.setLoopRangeByProgress(Math.min(trackLoopBar.inP, v - 0.02), v)
                                }
                            }
                            Label {
                                text: "%"
                                color: "#c5cede"
                                font.pixelSize: 11
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }
                    }

                    // EQ band columns: [field + slider + label] × 12, evenly across full width
                    RowLayout {
                        id: faderStrip
                        visible: rowRoot.fxOpen
                        Layout.fillWidth: true
                        Layout.preferredHeight: 148
                        spacing: 12

                        Repeater {
                            model: mixer.faderCount

                            ColumnLayout {
                                id: bandCol
                                required property int index
                                Layout.fillWidth: true
                                Layout.minimumWidth: 40
                                Layout.preferredWidth: 52
                                Layout.fillHeight: true
                                spacing: 2

                                readonly property bool isThr: index === 10
                                readonly property bool isRat: index === 11
                                readonly property real paramFrom: isThr ? 0.05 : (isRat ? 1.0 : -12)
                                readonly property real paramTo: isThr ? 1.0 : (isRat ? 20.0 : 12)
                                readonly property int paramDecimals: (isThr || isRat) ? 2 : 1

                                function currentValue() {
                                    if (!rowRoot.hasTrack)
                                        return isRat ? 3.0 : (isThr ? 0.75 : 0)
                                    if (isThr)
                                        return trackVm.compThreshold
                                    if (isRat)
                                        return trackVm.compRatio
                                    var bands = rowRoot.trackVm.eqBandDb
                                    if (!bands || bands.length <= index)
                                        return 0
                                    var v = bands[index]
                                    return (v === undefined || v === null) ? 0 : v
                                }
                                function applyValue(v) {
                                    if (!rowRoot.hasTrack)
                                        return
                                    if (isThr)
                                        trackVm.compThreshold = v
                                    else if (isRat)
                                        trackVm.compRatio = v
                                    else
                                        rowRoot.trackVm.setEqBandDb(index, v)
                                }

                                NumericValueField {
                                    Layout.fillWidth: true
                                    Layout.maximumWidth: 52
                                    Layout.preferredHeight: 22
                                    Layout.alignment: Qt.AlignHCenter
                                    from: bandCol.paramFrom
                                    to: bandCol.paramTo
                                    decimals: bandCol.paramDecimals
                                    percentScale: false
                                    value: bandCol.currentValue()
                                    onValueEdited: function(v) { bandCol.applyValue(v) }
                                }

                                Slider {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.alignment: Qt.AlignHCenter
                                    orientation: Qt.Vertical
                                    from: bandCol.paramFrom
                                    to: bandCol.paramTo
                                    value: bandCol.currentValue()
                                    onMoved: bandCol.applyValue(value)
                                }

                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 14
                                    text: bandCol.isThr ? "Thr" : (bandCol.isRat ? "Rat" : (bandCol.index + 1).toString())
                                    color: "#c5cede"
                                    font.pixelSize: 11
                                    font.bold: bandCol.isThr || bandCol.isRat
                                    horizontalAlignment: Text.AlignHCenter
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: trackList.count === 0
                text: "Library → select asset → Import Selected"
                color: "#c5cede"
                font.pixelSize: 14
            }
        }
    }
}
