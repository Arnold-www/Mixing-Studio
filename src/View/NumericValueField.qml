import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Compact editable numeric field; syncs from value unless focused.
TextField {
    id: root

    property real value: 0
    property real from: 0
    property real to: 1
    property int decimals: 0
    property bool percentScale: false // UI shows value*100; edits commit /100

    signal valueEdited(real newValue)

    Material.theme: Material.Dark
    Material.accent: Material.Teal
    color: "#ffffff"
    selectedTextColor: "#0a0c10"
    selectionColor: "#5dcea8"
    font.family: "Consolas"
    font.pixelSize: 11
    horizontalAlignment: TextInput.AlignHCenter
    selectByMouse: true
    leftPadding: 4
    rightPadding: 4
    topPadding: 2
    bottomPadding: 2
    implicitHeight: 26
    implicitWidth: decimals > 0 ? 52 : 44
    validator: DoubleValidator {
        bottom: root.percentScale ? root.from * 100.0 : root.from
        top: root.percentScale ? root.to * 100.0 : root.to
        decimals: Math.max(0, root.decimals)
        notation: DoubleValidator.StandardNotation
    }

    function toDisplay(v) {
        var d = root.percentScale ? v * 100.0 : v
        if (root.decimals <= 0)
            return Math.round(d).toString()
        return Number(d).toFixed(root.decimals)
    }
    function fromDisplay(text) {
        var n = parseFloat(text)
        if (isNaN(n))
            return root.value
        if (root.percentScale)
            n = n / 100.0
        return Math.max(root.from, Math.min(root.to, n))
    }
    function syncFromValue() {
        if (activeFocus)
            return
        var t = toDisplay(root.value)
        if (text !== t)
            text = t
    }
    function commit() {
        var v = fromDisplay(text)
        root.valueEdited(v)
        Qt.callLater(syncFromValue)
    }

    onValueChanged: syncFromValue()
    Component.onCompleted: syncFromValue()
    onEditingFinished: commit()
    Keys.onReturnPressed: function(event) { commit(); focus = false; event.accepted = true }
    Keys.onEnterPressed: function(event) { commit(); focus = false; event.accepted = true }

    background: Rectangle {
        radius: 3
        color: root.activeFocus ? "#1a2430" : "#151a22"
        border.width: 1
        border.color: root.activeFocus ? "#5dcea8" : "#2a3140"
    }
}
