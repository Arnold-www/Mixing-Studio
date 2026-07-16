import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs
import MixingStudio

Rectangle {
    id: bar
    color: "#12151c"
    border.color: "#2a3140"
    radius: 6

    Material.theme: Material.Dark
    Material.accent: Material.Teal
    Material.foreground: "#ffffff"

    property bool libraryOpen: false
    property bool projectsOpen: false
    property bool spectrumOpen: false

    FileDialog {
        id: audioDialog
        title: "Import Audio"
        nameFilters: ["Audio files (*.wav *.mp3)", "WAV (*.wav)", "MP3 (*.mp3)"]
        fileMode: FileDialog.OpenFiles
        onAccepted: {
            for (var i = 0; i < selectedFiles.length; ++i)
                mixerViewModel.importLocalFile(selectedFiles[i])
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        Label {
            text: "Mixing Studio"
            font.pixelSize: 15
            font.bold: true
            color: "#ffffff"
            Layout.rightMargin: 4
        }

        ToolIconButton {
            text: "Library"
            toggled: bar.libraryOpen
            onClicked: {
                bar.libraryOpen = !bar.libraryOpen
                if (bar.libraryOpen)
                    bar.projectsOpen = false
            }
        }
        ToolIconButton {
            text: "Projects"
            toggled: bar.projectsOpen
            onClicked: {
                bar.projectsOpen = !bar.projectsOpen
                if (bar.projectsOpen)
                    bar.libraryOpen = false
            }
        }
        ToolIconButton {
            text: "Spectrum"
            toggled: bar.spectrumOpen
            onClicked: bar.spectrumOpen = !bar.spectrumOpen
        }

        Item { Layout.preferredWidth: 6 }

        ToolIconButton { text: "Import"; onClicked: audioDialog.open() }
        ToolIconButton { text: "Demo"; onClicked: mixerViewModel.importMockTrack() }
        ToolIconButton { text: "Sample"; onClicked: mixerViewModel.loadSampleProject() }
        ToolIconButton { text: "Save"; onClicked: mixerViewModel.saveProject() }
        ToolIconButton { text: "Export"; onClicked: mixerViewModel.exportMix() }

        Item { Layout.preferredWidth: 6 }

        ToolIconButton {
            text: mixerViewModel.playing ? "Pause" : "Play"
            primary: true
            toggled: mixerViewModel.playing
            implicitWidth: 72
            onClicked: mixerViewModel.playing ? mixerViewModel.pause() : mixerViewModel.play()
        }
        ToolIconButton {
            text: "Stop"
            onClicked: mixerViewModel.stop()
        }

        Label {
            text: mixerViewModel.playbackTimeText
            color: "#ffffff"
            font.family: "Consolas"
            font.pixelSize: 13
            Layout.preferredWidth: 120
        }

        Item { Layout.fillWidth: true; Layout.minimumWidth: 8 }

        Label {
            text: "Master"
            color: "#ffffff"
            font.pixelSize: 12
            Layout.alignment: Qt.AlignVCenter
        }
        Slider {
            id: masterSlider
            from: 0.0
            to: 1.0
            onMoved: mixerViewModel.masterVolume = value
            Layout.preferredWidth: 110
            Layout.alignment: Qt.AlignVCenter
            Material.accent: Material.Teal
            Binding {
                target: masterSlider
                property: "value"
                value: mixerViewModel.masterVolume
                when: !masterSlider.pressed
            }
        }
        NumericValueField {
            Layout.preferredWidth: 44
            Layout.alignment: Qt.AlignVCenter
            from: 0.0
            to: 1.0
            decimals: 0
            percentScale: true
            value: mixerViewModel.masterVolume
            onValueEdited: function(v) { mixerViewModel.masterVolume = v }
        }
        Label {
            text: "%"
            color: "#c5cede"
            font.pixelSize: 11
            Layout.alignment: Qt.AlignVCenter
        }
        LevelMeterBar {
            Layout.preferredWidth: 140
            Layout.minimumWidth: 120
            Layout.preferredHeight: 12
            Layout.alignment: Qt.AlignVCenter
            currentLevel: mixerViewModel.vuLevel
            threshold: 0.85
        }
        ToolIconButton {
            text: "Mock"
            toggled: mixerViewModel.mockValidationMode
            Layout.alignment: Qt.AlignVCenter
            onClicked: mixerViewModel.mockValidationMode = !mixerViewModel.mockValidationMode
        }

        Item { Layout.preferredWidth: 32; Layout.maximumWidth: 32 }
    }
}
