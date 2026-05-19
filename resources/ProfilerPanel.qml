import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: profilerPanel
    property var editorEngine

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        Text {
            text: qsTr("Profiler")
            color: palette.text
            font.pixelSize: 13
            font.bold: true
        }

        GroupBox {
            title: qsTr("Frame Timing")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 4

                RowLayout {
                    Text { text: qsTr("Frame Time:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "0.00 ms"; font.pixelSize: 11; font.bold: true }
                }
                RowLayout {
                    Text { text: qsTr("FPS:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "60"; font.pixelSize: 11; font.bold: true }
                }
                RowLayout {
                    Text { text: qsTr("CPU Time:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "0.00 ms"; font.pixelSize: 11; font.bold: true }
                }
                RowLayout {
                    Text { text: qsTr("GPU Time:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "0.00 ms"; font.pixelSize: 11; font.bold: true }
                }
            }
        }

        GroupBox {
            title: qsTr("Render Stats")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 4

                RowLayout {
                    Text { text: qsTr("Draw Calls:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "0"; font.pixelSize: 11 }
                }
                RowLayout {
                    Text { text: qsTr("Triangles:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "0"; font.pixelSize: 11 }
                }
                RowLayout {
                    Text { text: qsTr("Shader Switches:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "0"; font.pixelSize: 11 }
                }
            }
        }

        GroupBox {
            title: qsTr("Memory")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 4

                RowLayout {
                    Text { text: qsTr("Total:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "0 MB"; font.pixelSize: 11 }
                }
                RowLayout {
                    Text { text: qsTr("Used:"); font.pixelSize: 11; Layout.preferredWidth: 100 }
                    Text { text: "0 MB"; font.pixelSize: 11 }
                }
                ProgressBar {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: 0
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
