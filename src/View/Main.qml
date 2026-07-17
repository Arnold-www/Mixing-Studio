import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MixingStudio

ApplicationWindow {
    id: window
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
            }

            // One stage card: timeline + waveform (shared chrome, no stacked frames)
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
                            text: mixerViewModel.playing ? "Waveform  ·  live" : "Waveform  ·  overview"
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        Label {
                            Layout.fillWidth: true
                            text: mixerViewModel.statusMessage
                            color: "#c5cede"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Label {
                            text: mixerViewModel.selectedTrackIndex >= 0
                                  ? ("Track " + (mixerViewModel.selectedTrackIndex + 1) + "  ·  click plot to edit automation")
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
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#2a3140"
                    }

                    // Wrapper owns Layout size; Waveform fills wrapper only (clip).
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        z: 0

                        WaveformPanel {
                            anchors.fill: parent
                        }
                    }
                }
            }

            // Track mixer — full-width rows under waveform
            TrackMixerList {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 220
            }
        }

        // Left drawer: Library / Projects (VS Code–style side panel)
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
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    active: leftDrawer.width > 40
                    sourceComponent: transport.libraryOpen
                                     ? libraryComp
                                     : (transport.projectsOpen ? projectsComp : null)
                }
            }
        }

        // Right drawer: Spectrum (optional, not crowding main desk)
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
                    onCloseRequested: transport.spectrumOpen = false
                }
            }
        }
    }

    Component {
        id: libraryComp
        LibraryPanel {}
    }
    Component {
        id: projectsComp
        ProjectsPanel {}
    }
}
