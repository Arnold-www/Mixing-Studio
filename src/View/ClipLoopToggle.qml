import QtQuick
import QtQuick.Controls

// Segmented Clip | Loop control. Avoids Material Button indicator overflow.
Item {
    id: root
    implicitWidth: 96
    implicitHeight: 30

    property bool loopEnabled: false
    signal toggled()

    Rectangle {
        id: frame
        anchors.fill: parent
        radius: 5
        color: "#0c1016"
        border.width: 1
        border.color: root.loopEnabled ? "#5dcea8" : "#3a4458"
        clip: true

        // Dark state thumb — inset so it never breaks the frame border
        Rectangle {
            id: thumb
            width: Math.max(8, (frame.width - 6) / 2)
            height: Math.max(4, parent.height - 6)
            anchors.verticalCenter: parent.verticalCenter
            x: root.loopEnabled ? (frame.width - width - 3) : 3
            radius: 3
            color: root.loopEnabled ? "#2a5c52" : "#252b38"
            border.width: 0
            z: 0

            Behavior on x {
                NumberAnimation { duration: 120; easing.type: Easing.OutCubic }
            }
        }

        Row {
            anchors.fill: parent
            anchors.margins: 2
            z: 2

            Item {
                width: parent.width / 2
                height: parent.height
                Text {
                    anchors.centerIn: parent
                    text: "Clip"
                    font.pixelSize: 11
                    font.bold: !root.loopEnabled
                    color: !root.loopEnabled ? "#ffffff" : "#8a96a8"
                    z: 3
                }
            }
            Item {
                width: parent.width / 2
                height: parent.height
                Text {
                    anchors.centerIn: parent
                    text: "Loop"
                    font.pixelSize: 11
                    font.bold: root.loopEnabled
                    color: root.loopEnabled ? "#ffffff" : "#8a96a8"
                    z: 3
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            z: 4
            onClicked: root.toggled()
        }
    }
}
