import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import MixingStudio

// Optional dock panel: collapsed = one clear toolbar row; no Layout self-attachment.
Item {
    id: host

    property string title: "Panel"
    property string panelId: ""
    property bool expanded: false
    property bool floating: false
    property Component content: null
    property int floatWidth: 640
    property int floatHeight: 420

    readonly property bool showingDock: expanded && !floating

    // --- Collapsed: one full-width bar, buttons not cramped ---
    Rectangle {
        id: collapsedBar
        anchors.fill: parent
        visible: !host.expanded && !host.floating
        color: "#12151c"
        border.color: "#2a3140"
        radius: 5

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 8
            spacing: 10

            Label {
                text: host.title
                color: "#9aa6b8"
                font.pixelSize: 12
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            DarkButton {
                text: "Open"
                Layout.preferredWidth: 72
                onClicked: host.expanded = true
            }

            DarkButton {
                text: "Pop out"
                Layout.preferredWidth: 88
                onClicked: {
                    host.floating = true
                    host.expanded = false
                }
            }
        }
    }

    // --- Expanded dock ---
    Rectangle {
        anchors.fill: parent
        visible: host.showingDock
        color: "#12151c"
        border.color: "#2a3140"
        radius: 5
        clip: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                color: "#181c26"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 8
                    spacing: 10

                    Label {
                        text: host.title
                        color: "#e6ebf2"
                        font.bold: true
                        font.pixelSize: 12
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    DarkButton {
                        text: "Fold"
                        Layout.preferredWidth: 72
                        onClicked: host.expanded = false
                    }

                    DarkButton {
                        text: "Pop out"
                        Layout.preferredWidth: 88
                        onClicked: {
                            host.floating = true
                            host.expanded = false
                        }
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true
                active: host.showingDock && host.content !== null
                sourceComponent: host.content
            }
        }
    }

    // --- Floated reclaim strip ---
    Rectangle {
        anchors.fill: parent
        visible: host.floating
        color: "#0e1117"
        border.color: "#3d9b84"
        radius: 5

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 8
            spacing: 10

            Label {
                text: host.title + "  ·  floated"
                color: "#3d9b84"
                font.pixelSize: 12
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            DarkButton {
                text: "Dock back"
                primary: true
                Layout.preferredWidth: 100
                onClicked: {
                    host.floating = false
                    host.expanded = true
                }
            }
        }
    }

    Window {
        id: floatWindow
        title: host.title + " — Mixing Studio"
        width: host.floatWidth
        height: host.floatHeight
        minimumWidth: 400
        minimumHeight: 280
        visible: host.floating
        color: "#0a0c10"

        Material.theme: Material.Dark
        Material.accent: Material.Teal
        Material.background: "#0a0c10"
        Material.foreground: "#e6ebf2"

        onClosing: function(close) {
            close.accepted = true
            host.floating = false
            host.expanded = false
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 8
            color: "#12151c"
            border.color: "#2a3140"
            radius: 5
            clip: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    color: "#181c26"
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 10
                        Label {
                            text: host.title
                            color: "#e6ebf2"
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        DarkButton {
                            text: "Dock"
                            Layout.preferredWidth: 72
                            onClicked: {
                                host.floating = false
                                host.expanded = true
                            }
                        }
                    }
                }

                Loader {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    active: host.floating && host.content !== null
                    sourceComponent: host.content
                }
            }
        }
    }

    function closeFloating() {
        floating = false
    }
}
