import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import MixingStudio

Button {
    id: control

    property bool primary: false

    flat: true
    font.pixelSize: 12
    font.bold: primary
    implicitHeight: 32
    implicitWidth: Math.max(72, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: 12
    rightPadding: 12
    topPadding: 6
    bottomPadding: 6

    // Force Material to use our colors (otherwise Light style paints white chrome).
    Material.theme: Material.Dark
    Material.elevation: 0
    Material.background: primary ? "#2f8f78" : "#2a3142"
    Material.foreground: "#f2f6fb"

    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? "#6b7a8c"
               : (control.primary ? "#ffffff" : "#f2f6fb")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitHeight: 32
        radius: 5
        color: {
            if (!control.enabled)
                return "#161a22"
            if (control.primary) {
                if (control.down)
                    return "#245a4d"
                if (control.hovered)
                    return "#38a88c"
                return "#2f8f78"
            }
            if (control.down)
                return "#1a2030"
            if (control.hovered)
                return "#343c4e"
            return "#2a3142"
        }
        border.width: control.primary ? 0 : 1
        border.color: control.enabled ? "#3a4458" : "#2a3140"
    }
}
