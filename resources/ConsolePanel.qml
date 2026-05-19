import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: consolePanel
    property var editorEngine

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: qsTr("Console")
                color: palette.text
                font.pixelSize: 13
                font.bold: true
            }
            Item { Layout.fillWidth: true }

            ToolButton {
                text: qsTr("Clear")
                onClicked: {
                    logListModel.clear()
                    if (editorEngine) editorEngine.clear_logs()
                }
            }
            ComboBox {
                model: [qsTr("All"), qsTr("Info"), qsTr("Warning"), qsTr("Error")]
                currentIndex: 0
            }
        }

        ListView {
            id: logListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: ListModel {
                id: logListModel
            }

            delegate: ItemDelegate {
                width: logListView.width
                height: 24

                contentItem: RowLayout {
                    spacing: 6
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: model.level === 0 ? "#4CAF50" :
                               model.level === 1 ? "#2196F3" :
                               model.level === 2 ? "#FF9800" :
                               model.level === 3 ? "#F44336" :
                               model.level === 4 ? "#9C27B0" : "#666"
                    }
                    Text {
                        text: model.message
                        color: palette.text
                        font.pixelSize: 11
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }

            function appendLogEntry(message, level) {
                logListModel.append({message: message, level: level})
            }
        }
    }
}
