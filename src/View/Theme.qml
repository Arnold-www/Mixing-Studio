pragma Singleton
import QtQuick

QtObject {
    // Ableton / Reaper style charcoal night desk
    readonly property color windowBg: "#0a0c10"
    readonly property color panelBg: "#12151c"
    readonly property color panelAlt: "#181c26"
    readonly property color panelRaised: "#1e2430"
    readonly property color railBg: "#0e1117"
    readonly property color border: "#2a3140"
    readonly property color borderStrong: "#3d9b84"
    readonly property color textPrimary: "#e6ebf2"
    readonly property color textSecondary: "#9aa6b8"
    readonly property color textMuted: "#6a7588"
    readonly property color accent: "#3d9b84"
    readonly property color accentSoft: "#245a4d"
    readonly property color danger: "#d45d5d"
    readonly property color warn: "#d4a05d"
    readonly property color meterTrack: "#151a22"
    readonly property color waveFill: "#3d9b84"
    readonly property color waveStroke: "#5dcea8"
    readonly property color autoLine: "#e0a45a"
    readonly property color playhead: "#e07a5f"

    readonly property color buttonBg: "#252b38"
    readonly property color buttonFg: "#f0f4fa"
    readonly property color buttonBorder: "#3a4458"
    readonly property color buttonPrimaryBg: "#2f8f78"
    readonly property color buttonPrimaryFg: "#ffffff"
    readonly property color buttonDisabledBg: "#161a22"
    readonly property color buttonDisabledFg: "#5a6578"
    readonly property color placeholderBg: "#0c0f14"
    readonly property color listHover: "#222836"
    readonly property color listSelected: "#1a3d36"

    readonly property int radius: 6
    readonly property int gap: 8
    readonly property int pad: 10
    readonly property int collapsedBar: 32
    readonly property int browserWidth: 380
    readonly property int analysisHeight: 260
}
