import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MixingStudio

// Pure View root. MixingStudioViewBinder connects root signals/properties.
// No ViewModel injection / ContextProperty / setInitialProperties.
ApplicationWindow {
    id: window
    objectName: "mainWindow"
    width: 1480
    height: 920
    minimumWidth: 1100
    minimumHeight: 700
    visible: true
    title: "Mixing Studio"
    color: "#0a0c10"

    Material.theme: Material.Dark
    Material.accent: Material.Teal
    Material.background: "#0a0c10"
    Material.foreground: "#e6ebf2"
    Material.elevation: 0

    // --- Inbound UI state (Binder setProperty ↔ViewModel) ---
    property bool playing: false
    property string playbackTimeText: ""
    property real masterVolume: 1.0
    property real vuLevel: 0.0
    property bool mockValidationMode: false

    property real playbackProgress: 0
    property bool loopEnabled: false
    property real loopStartProgress: 0
    property real loopEndProgress: 1

    property var waveformPoints: []
    property var automationPoints: []
    property var spectrumLevels: []

    property var tracks: []
    property int selectedTrackIndex: -1
    property bool anySolo: false

    property string statusMessage: ""

    property string assetSearchText: ""
    property var assetNames: []
    property int selectedAssetIndex: -1

    property var projectNames: []
    property int selectedRecentProjectIndex: -1

    // --- Outbound user actions (Binder connect ↔ViewModel) ---
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

    signal seekRequested(real progress)
    signal loopRangeRequested(real startProgress, real endProgress)
    signal loopEnabledToggled()

    signal addAutomationRequested(real progress, real value)
    signal moveAutomationRequested(int pointIndex, real progress, real value)

    signal trackSelected(int index)
    signal clearAutomationRequested()
    signal deleteTrackRequested()

    signal searchTextEdited(string text)
    signal assetIndexSelected(int index)
    signal importAssetRequested()

    signal projectIndexSelected(int index)
    signal restoreProjectRequested()
    signal deleteProjectRequested()

    background: Rectangle {
        anchors.fill: parent
        color: "#0a0c10"
    }

    Rectangle {
        anchors.fill: parent
        color: "#0a0c10"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            TransportBar {
                id: transport
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                Layout.maximumHeight: 52

                playing: window.playing
                playbackTimeText: window.playbackTimeText
                masterVolume: window.masterVolume
                vuLevel: window.vuLevel
                mockValidationMode: window.mockValidationMode

                onImportFilesRequested: (urls) => window.importFilesRequested(urls)
                onDemoRequested: window.demoRequested()
                onSampleRequested: window.sampleRequested()
                onSaveRequested: window.saveRequested()
                onExportRequested: window.exportRequested()
                onPlayRequested: window.playRequested()
                onPauseRequested: window.pauseRequested()
                onStopRequested: window.stopRequested()
                onMasterVolumeEdited: (value) => window.masterVolumeEdited(value)
                onMockToggled: window.mockToggled()
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                Layout.minimumHeight: 220
                Layout.maximumHeight: 360
                color: "#12151c"
                border.color: "#2a3140"
                radius: 6
                clip: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        Label {
                            text: window.playing ? "Waveform  ·  live" : "Waveform  ·  overview"
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        Label {
                            Layout.fillWidth: true
                            text: window.statusMessage
                            color: "#c5cede"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Label {
                            text: window.selectedTrackIndex >= 0
                                  ? ("Track " + (window.selectedTrackIndex + 1)
                                     + "  ·  click plot to edit automation")
                                  : "Select a track to edit automation"
                            color: "#c5cede"
                            font.pixelSize: 12
                        }
                    }

                    TimelineStrip {
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight
                        Layout.minimumHeight: 40
                        Layout.maximumHeight: 64
                        z: 2

                        playbackProgress: window.playbackProgress
                        loopEnabled: window.loopEnabled
                        loopStartProgress: window.loopStartProgress
                        loopEndProgress: window.loopEndProgress

                        onSeekRequested: (progress) => window.seekRequested(progress)
                        onLoopRangeRequested: (startProgress, endProgress) =>
                            window.loopRangeRequested(startProgress, endProgress)
                        onLoopEnabledToggled: window.loopEnabledToggled()
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#2a3140"
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        z: 0

                        WaveformPanel {
                            anchors.fill: parent
                            playing: window.playing
                            playbackProgress: window.playbackProgress
                            waveformPoints: window.waveformPoints
                            automationPoints: window.automationPoints
                            selectedTrackIndex: window.selectedTrackIndex

                            onAddAutomationRequested: (progress, value) =>
                                window.addAutomationRequested(progress, value)
                            onMoveAutomationRequested: (pointIndex, progress, value) =>
                                window.moveAutomationRequested(pointIndex, progress, value)
                        }
                    }
                }
            }

            TrackMixerList {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 220

                tracks: window.tracks
                selectedTrackIndex: window.selectedTrackIndex
                anySolo: window.anySolo

                onTrackSelected: (index) => window.trackSelected(index)
                onClearAutomationRequested: window.clearAutomationRequested()
                onDeleteTrackRequested: window.deleteTrackRequested()
            }
        }

        Rectangle {
            id: leftDrawer
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 12
            anchors.topMargin: 76
            width: (transport.libraryOpen || transport.projectsOpen) ? 400 : 0
            visible: width > 0
            color: "#12151c"
            border.color: "#3d9b84"
            radius: 6
            clip: true
            z: 20

            Behavior on width {
                NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6
                visible: leftDrawer.width > 40

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: transport.libraryOpen ? "Library" : "Projects"
                        color: "#ffffff"
                        font.bold: true
                        font.pixelSize: 14
                        Layout.fillWidth: true
                    }
                    ToolIconButton {
                        text: "Close"
                        onClicked: {
                            transport.libraryOpen = false
                            transport.projectsOpen = false
                        }
                    }
                }

                Loader {
                    id: sidePanelLoader
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    active: leftDrawer.width > 40
                    sourceComponent: transport.libraryOpen
                                     ? libraryComp
                                     : (transport.projectsOpen ? projectsComp : null)
                }
            }
        }

        Rectangle {
            id: rightDrawer
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 12
            anchors.topMargin: 76
            width: transport.spectrumOpen ? 420 : 0
            visible: width > 0
            color: "#12151c"
            border.color: "#3d9b84"
            radius: 6
            clip: true
            z: 20

            Behavior on width {
                NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
            }

            Item {
                anchors.fill: parent
                anchors.margins: 10
                visible: rightDrawer.width > 40
                clip: true

                SpectrumPanel {
                    anchors.fill: parent
                    showCloseButton: true
                    playing: window.playing
                    spectrumLevels: window.spectrumLevels
                    onCloseRequested: transport.spectrumOpen = false
                }
            }
        }
    }

    Component {
        id: libraryComp
        LibraryPanel {
            searchText: window.assetSearchText
            assetNames: window.assetNames
            selectedIndex: window.selectedAssetIndex
            onSearchTextEdited: (text) => window.searchTextEdited(text)
            onAssetIndexSelected: (index) => window.assetIndexSelected(index)
            onImportRequested: window.importAssetRequested()
        }
    }
    Component {
        id: projectsComp
        ProjectsPanel {
            projectNames: window.projectNames
            selectedIndex: window.selectedRecentProjectIndex
            onProjectIndexSelected: (index) => window.projectIndexSelected(index)
            onRestoreRequested: window.restoreProjectRequested()
            onDeleteRequested: window.deleteProjectRequested()
            onLoadSampleRequested: window.sampleRequested()
            onSaveRequested: window.saveRequested()
            onExportRequested: window.exportRequested()
        }
    }
}
