import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MixingStudio

Item {
    id: root
    anchors.fill: parent

    property var projectNames: []
    property int selectedIndex: -1

    signal projectIndexSelected(int index)
    signal restoreRequested()
    signal deleteRequested()
    signal loadSampleRequested()
    signal saveRequested()
    signal exportRequested()

    Material.theme: Material.Dark
    Material.accent: Material.Teal
    Material.background: "#12151c"
    Material.foreground: "#ffffff"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: "Recent projects  ·  select then Restore"
            color: "#c5cede"
            font.pixelSize: 12
        }

        ListView {
            id: recentProjectList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: root.projectNames
            currentIndex: root.selectedIndex
            onCountChanged: {
                if (root.selectedIndex >= count)
                    root.projectIndexSelected(-1)
            }

            delegate: Rectangle {
                id: row
                width: ListView.view.width
                height: 36
                radius: 5
                property bool selected: root.selectedIndex === index
                property bool hovered: rowMouse.containsMouse

                color: {
                    if (selected)
                        return "#2a5c52"
                    if (hovered)
                        return "#3a465c"
                    return "#1e2430"
                }
                border.color: selected ? "#5dcea8" : (hovered ? "#6a7a94" : "#2a3140")
                border.width: 1

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    text: modelData
                    color: "#ffffff"
                    font.pixelSize: 13
                    font.bold: row.selected
                    elide: Text.ElideMiddle
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.projectIndexSelected(index)
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 8
            rowSpacing: 8

            ToolIconButton {
                text: "Restore"
                enabled: root.selectedIndex >= 0
                Layout.fillWidth: true
                onClicked: root.restoreRequested()
            }
            ToolIconButton {
                text: "Delete"
                enabled: root.selectedIndex >= 0
                Layout.fillWidth: true
                onClicked: root.deleteRequested()
            }
            ToolIconButton {
                text: "Load Sample"
                primary: true
                Layout.fillWidth: true
                Layout.columnSpan: 2
                onClicked: root.loadSampleRequested()
            }
            ToolIconButton {
                text: "Save"
                Layout.fillWidth: true
                onClicked: root.saveRequested()
            }
            ToolIconButton {
                text: "Export WAV"
                Layout.fillWidth: true
                onClicked: root.exportRequested()
            }
        }
    }
}
