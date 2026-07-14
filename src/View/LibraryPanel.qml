import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    Layout.preferredWidth: 244
    Layout.fillHeight: true
    radius: 8
    color: "#ffffff"
    border.color: "#d6dde7"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        Label {
            text: "Projects"
            color: "#253241"
            font.bold: true
        }

        ListView {
            id: recentProjectList
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            clip: true
            spacing: 4
            model: mixerViewModel.recentProjectNames
            onCountChanged: {
                if (currentIndex >= count)
                    currentIndex = -1
            }

            delegate: ItemDelegate {
                width: ListView.view.width
                text: modelData
                highlighted: ListView.isCurrentItem
                onClicked: recentProjectList.currentIndex = index
            }
        }

        Button {
            text: "Restore Selected"
            enabled: recentProjectList.currentIndex >= 0
            onClicked: mixerViewModel.restoreRecentProject(mixerViewModel.recentProjectNames[recentProjectList.currentIndex])
            Layout.fillWidth: true
        }

        Button {
            text: "Save Snapshot"
            onClicked: mixerViewModel.saveMockProject()
            Layout.fillWidth: true
        }

        Label {
            text: "Library"
            color: "#253241"
            font.bold: true
        }

        TextField {
            text: mixerViewModel.assetSearchText
            placeholderText: "Search assets"
            selectByMouse: true
            onTextEdited: mixerViewModel.assetSearchText = text
            Layout.fillWidth: true
        }

        ListView {
            id: assetList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: mixerViewModel.filteredAssetNames
            onCountChanged: {
                if (currentIndex >= count)
                    currentIndex = -1
            }

            delegate: ItemDelegate {
                width: ListView.view.width
                text: modelData
                highlighted: ListView.isCurrentItem
                onClicked: assetList.currentIndex = index
            }
        }

        Button {
            text: "Import Selected"
            enabled: assetList.currentIndex >= 0
            onClicked: mixerViewModel.importAssetByName(mixerViewModel.filteredAssetNames[assetList.currentIndex])
            Layout.fillWidth: true
        }
    }
}
