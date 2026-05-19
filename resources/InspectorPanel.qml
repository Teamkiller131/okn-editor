import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: inspectorPanel
    property var editorEngine

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        Text {
            text: qsTr("Inspector")
            color: palette.text
            font.pixelSize: 13
            font.bold: true
        }

        Label {
            text: qsTr("No entity selected")
            color: palette.text
            font.pixelSize: 11
            Layout.fillWidth: true
            Layout.fillHeight: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            visible: true
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            visible: false

            ColumnLayout {
                width: parent ? parent.width : 280
                spacing: 8

                GroupBox {
                    title: qsTr("Transform")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 4

                        RowLayout {
                            Text { text: "X:"; Layout.preferredWidth: 20; font.pixelSize: 11 }
                            SpinBox { id: posX; from: -999999; to: 999999; value: 0; Layout.fillWidth: true }
                        }
                        RowLayout {
                            Text { text: "Y:"; Layout.preferredWidth: 20; font.pixelSize: 11 }
                            SpinBox { id: posY; from: -999999; to: 999999; value: 0; Layout.fillWidth: true }
                        }
                        RowLayout {
                            Text { text: "Z:"; Layout.preferredWidth: 20; font.pixelSize: 11 }
                            SpinBox { id: posZ; from: -999999; to: 999999; value: 0; Layout.fillWidth: true }
                        }
                    }
                }

                GroupBox {
                    title: qsTr("Components")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 4

                        Repeater {
                            model: []
                            delegate: CheckBox {
                                text: modelData
                                checked: true
                            }
                        }
                    }
                }
            }
        }
    }
}
