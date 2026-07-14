import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1180
    height: 700
    minimumWidth: 980
    minimumHeight: 580
    visible: true
    title: "Mixing Studio"
    color: "#eef2f5"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        TransportBar {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 168
            spacing: 8

            WaveformPanel { }
            SpectrumPanel { }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 300
            spacing: 8

            LibraryPanel { }
            TrackMixerList { }
        }
    }
}
