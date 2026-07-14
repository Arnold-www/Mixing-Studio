import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    Layout.fillWidth: true
    Layout.preferredHeight: 82
    radius: 8
    color: "#ffffff"
    border.color: "#d6dde7"

    GridLayout {
        anchors.fill: parent
        anchors.margins: 10
        columns: 8
        columnSpacing: 10
        rowSpacing: 8

        Label {
            text: "Mixing Studio"
            font.pixelSize: 22
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
