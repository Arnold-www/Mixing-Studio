import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MixingStudio

Item {
    id: root
    anchors.fill: parent

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
            model: mixerViewModel.recentProjectNames
            onCountChanged: {
                if (currentIndex >= count)
                    currentIndex = -1
            }

            delegate: Rectangle {
                id: row
                width: ListView.view.width
                height: 36
                radius: 5
                property bool selected: recentProjectList.currentIndex === index
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
                    onClicked: recentProjectList.currentIndex = index
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
                enabled: recentProjectList.currentIndex >= 0
                Layout.fillWidth: true
                onClicked: mixerViewModel.restoreRecentProject(mixerViewModel.recentProjectNames[recentProjectList.currentIndex])
            }
            ToolIconButton {
                text: "Delete"
                enabled: recentProjectList.currentIndex >= 0
                Layout.fillWidth: true
                onClicked: mixerViewModel.deleteRecentProject(mixerViewModel.recentProjectNames[recentProjectList.currentIndex])
            }
            ToolIconButton {
                text: "Load Sample"
                primary: true
                Layout.fillWidth: true
                Layout.columnSpan: 2
                onClicked: mixerViewModel.loadSampleProject()
            }
            ToolIconButton {
                text: "Save"
                Layout.fillWidth: true
                onClicked: mixerViewModel.saveProject()
            }
            ToolIconButton {
                text: "Export WAV"
                Layout.fillWidth: true
                onClicked: mixerViewModel.exportMix()
            }
        }
    }
}
