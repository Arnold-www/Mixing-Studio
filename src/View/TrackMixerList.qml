import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    radius: 8
    color: "#ffffff"
    border.color: "#d6dde7"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Mixer"
                color: "#253241"
                font.bold: true
                Layout.fillWidth: true
            }

            Label {
                text: mixerViewModel.anySolo ? "Solo active" : "All tracks"
                color: mixerViewModel.anySolo ? "#b34d4d" : "#6b7280"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: "#f8fafc"
            border.color: "#e2e8f0"

            ListView {
                id: trackStripList
                anchors.fill: parent
                anchors.margins: 8
                orientation: ListView.Horizontal
                spacing: 8
                clip: true
                model: mixerViewModel.tracks

                delegate: Rectangle {
                    width: 166
                    height: trackStripList.height
                    radius: 8
                    color: modelData.audible ? "#ffffff" : "#eef1f5"
                    border.color: modelData.solo ? "#2f7d6d" : "#d6dde7"
                    opacity: modelData.audible ? 1.0 : 0.74

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 6

                        Label {
                            text: modelData.name
                            color: "#18202a"
                            font.bold: true
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 10

                            Rectangle {
                                Layout.preferredWidth: 24
                                Layout.fillHeight: true
                                radius: 4
                                color: "#e5eaf1"

                                Rectangle {
                                    width: parent.width
                                    height: Math.max(2, parent.height * modelData.meterLevel)
                                    anchors.bottom: parent.bottom
                                    radius: 4
                                    color: modelData.meterLevel > 0.76 ? "#b34d4d" : "#2f7d6d"
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: 8

                                Slider {
                                    orientation: Qt.Vertical
                                    from: 0.0
                                    to: 1.0
                                    value: modelData.volume
                                    onMoved: modelData.volume = value
                                    Layout.fillHeight: true
                                    Layout.alignment: Qt.AlignHCenter
                                }

                                Label {
                                    text: modelData.volumeText
                                    color: "#374151"
                                    font.family: "Menlo"
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Label {
                                text: "Pan"
                                color: "#4b5563"
                                Layout.preferredWidth: 30
                            }

                            Slider {
                                from: -1.0
                                to: 1.0
                                value: modelData.pan
                                onMoved: modelData.pan = value
                                Layout.fillWidth: true
                            }

                            Label {
                                text: modelData.panText
                                color: "#374151"
                                font.family: "Menlo"
                                horizontalAlignment: Text.AlignRight
                                Layout.preferredWidth: 36
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            CheckBox {
                                text: "M"
                                checked: modelData.muted
                                onToggled: modelData.muted = checked
                                Layout.fillWidth: true
                            }

                            CheckBox {
                                text: "S"
                                checked: modelData.solo
                                onToggled: modelData.solo = checked
                                Layout.fillWidth: true
                            }
                        }

                        Label {
                            text: modelData.meterText
                            color: "#4b5563"
                            font.family: "Menlo"
                            horizontalAlignment: Text.AlignHCenter
                            Layout.fillWidth: true
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: trackStripList.count === 0
                    text: "Import or select an asset to create mixer channels."
                    color: "#6b7280"
                }
            }
        }
    }
}
