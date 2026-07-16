import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Toolbar / chip control: hover lights up, click stays lit when toggled
Button {
    id: control

    property bool toggled: false
    property bool primary: false

    flat: true
    checkable: false
    hoverEnabled: true
    implicitWidth: Math.max(52, contentItem.implicitWidth + 20)
    implicitHeight: 30
    leftPadding: 10
    rightPadding: 10
    font.pixelSize: 12
    font.bold: toggled || primary || hovered

    Material.theme: Material.Dark
    Material.elevation: 0
    Material.background: "transparent"
    Material.foreground: "#ffffff"

    // Suppress Material's built-in indicator (can overflow custom chips).
    indicator: Item { width: 0; height: 0; visible: false }

    contentItem: Text {
        text: control.text
        font: control.font
        color: {
            if (!control.enabled)
                return "#6b7a8c"
            if (control.primary)
                return "#ffffff"
            if (control.toggled)
                return "#ffffff"
            if (control.hovered)
                return "#ffffff"
            return "#e8eef6"
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: 5
        color: {
            if (!control.enabled)
                return "#141820"
            if (control.primary) {
                if (control.down)
                    return "#245a4d"
                if (control.hovered)
                    return "#3aab8f"
                return "#2f8f78"
            }
            if (control.toggled)
                return "#2a5c52"
            if (control.down)
                return "#2a3548"
            if (control.hovered)
                return "#3a465c"
            return "#252b38"
        }
        border.width: control.toggled || control.hovered ? 1 : 1
        border.color: {
            if (control.toggled)
                return "#5dcea8"
            if (control.hovered)
                return "#6a7a94"
            return "#3a4458"
        }
    }
}
