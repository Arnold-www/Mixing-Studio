import QtQuick

// Simple level meter with hearing-protection / overload red zone.
Item {
    id: root

    property real currentLevel: 0.0
    property double threshold: 0.85
    property color safeColor: "#3fb59f"
    property color warnColor: "#ef4444"
    property color trackColor: "#151a22"
    property color borderColor: "#2a3140"

    implicitWidth: 120
    implicitHeight: 14

    readonly property real clamped: Math.max(0, Math.min(1, currentLevel))

    Rectangle {
        anchors.fill: parent
        radius: 3
        color: root.trackColor
        border.color: root.borderColor
        clip: true

        Rectangle {
            width: Math.max(2, parent.width * root.clamped)
            height: parent.height
            radius: 3
            color: root.currentLevel > root.threshold ? root.warnColor : root.safeColor
        }
    }
}
