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

    property bool playing: false
    property string playbackTimeText: ""
    property real masterVolume: 1.0
    property real vuLevel: 0.0
    property bool mockValidationMode: false

    signal importFilesRequested(var urls)
    signal demoRequested()
    signal sampleRequested()
    signal saveRequested()
    signal exportRequested()
    signal playRequested()
    signal pauseRequested()
    signal stopRequested()
    signal masterVolumeEdited(real value)
    signal mockToggled()

    FileDialog {
        id: audioDialog
        title: "Import Audio"
        nameFilters: ["Audio files (*.wav *.mp3)", "WAV (*.wav)", "MP3 (*.mp3)"]
        fileMode: FileDialog.OpenFiles
        onAccepted: bar.importFilesRequested(selectedFiles)
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
            objectName: "libraryButton"
            text: "Library"
            toggled: bar.libraryOpen
            onClicked: {
                bar.libraryOpen = !bar.libraryOpen
                if (bar.libraryOpen)
                    bar.projectsOpen = false
            }
        }
        ToolIconButton {
            objectName: "projectsButton"
            text: "Projects"
            toggled: bar.projectsOpen
            onClicked: {
                bar.projectsOpen = !bar.projectsOpen
                if (bar.projectsOpen)
                    bar.libraryOpen = false
            }
        }
        ToolIconButton {
            objectName: "spectrumButton"
            text: "Spectrum"
            toggled: bar.spectrumOpen
            onClicked: bar.spectrumOpen = !bar.spectrumOpen
        }

        Item { Layout.preferredWidth: 6 }

        ToolIconButton { objectName: "importButton"; text: "Import"; onClicked: audioDialog.open() }
        ToolIconButton { objectName: "demoButton"; text: "Demo"; onClicked: bar.demoRequested() }
        ToolIconButton { objectName: "sampleButton"; text: "Sample"; onClicked: bar.sampleRequested() }
        ToolIconButton { objectName: "saveButton"; text: "Save"; onClicked: bar.saveRequested() }
        ToolIconButton { objectName: "exportButton"; text: "Export"; onClicked: bar.exportRequested() }

        Item { Layout.preferredWidth: 6 }

        ToolIconButton {
            objectName: "playPauseButton"
            text: bar.playing ? "Pause" : "Play"
            primary: true
            toggled: bar.playing
            implicitWidth: 72
            onClicked: bar.playing ? bar.pauseRequested() : bar.playRequested()
        }
        ToolIconButton {
            objectName: "stopButton"
            text: "Stop"
            onClicked: bar.stopRequested()
        }

        Label {
            text: bar.playbackTimeText
            color: "#ffffff"
            font.family: Qt.platform.os === "osx" ? "Menlo" : "Consolas"
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
            objectName: "masterVolumeSlider"
            from: 0.0
            to: 1.0
            onMoved: bar.masterVolumeEdited(value)
            Layout.preferredWidth: 110
            Layout.alignment: Qt.AlignVCenter
            Material.accent: Material.Teal
            Binding {
                target: masterSlider
                property: "value"
                value: bar.masterVolume
                when: !masterSlider.pressed
            }
        }
        NumericValueField {
            objectName: "masterVolumeField"
            Layout.preferredWidth: 44
            Layout.alignment: Qt.AlignVCenter
            from: 0.0
            to: 1.0
            decimals: 0
            percentScale: true
            value: bar.masterVolume
            onValueEdited: function(v) { bar.masterVolumeEdited(v) }
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
            currentLevel: bar.vuLevel
            threshold: 0.85
        }
        ToolIconButton {
            objectName: "mockButton"
            text: "Mock"
            toggled: bar.mockValidationMode
            Layout.alignment: Qt.AlignVCenter
            onClicked: bar.mockToggled()
        }

        Item { Layout.preferredWidth: 32; Layout.maximumWidth: 32 }
    }
}
