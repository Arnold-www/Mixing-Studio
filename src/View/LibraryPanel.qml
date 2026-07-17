import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import MixingStudio

Item {
    id: root
    anchors.fill: parent

    property string searchText: ""
    property var assetNames: []
    property int selectedIndex: -1

    signal searchTextEdited(string text)
    signal assetIndexSelected(int index)
    signal importRequested()

    Material.theme: Material.Dark
    Material.accent: Material.Teal
    Material.background: "#12151c"
    Material.foreground: "#ffffff"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        TextField {
            id: searchField
            text: root.searchText
            placeholderText: "Search assets…"
            selectByMouse: true
            onTextEdited: root.searchTextEdited(text)
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: "#ffffff"
            placeholderTextColor: "#8a96a8"
            leftPadding: 10
            rightPadding: 10
            font.pixelSize: 13

            background: Rectangle {
                color: "#1e2430"
                border.color: searchField.activeFocus ? "#5dcea8" : "#3a4458"
                border.width: 1
                radius: 5
            }
        }

        Label {
            text: (root.assetNames ? root.assetNames.length : 0)
                  + " assets  ·  select then Import"
            color: "#c5cede"
            font.pixelSize: 12
        }

        ListView {
            id: assetList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: root.assetNames
            currentIndex: root.selectedIndex
            onCountChanged: {
                if (root.selectedIndex >= count)
                    root.assetIndexSelected(-1)
            }

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

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
                    onClicked: root.assetIndexSelected(index)
                }
            }
        }

        ToolIconButton {
            text: root.selectedIndex >= 0
                  ? ("Import  ·  " + root.assetNames[root.selectedIndex])
                  : "Import Selected"
            primary: true
            enabled: root.selectedIndex >= 0
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            onClicked: root.importRequested()
        }
    }
}
