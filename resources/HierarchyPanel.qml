import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: hierarchyPanel
    property var editorEngine

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: qsTr("Hierarchy")
                color: palette.text
                font.pixelSize: 13
                font.bold: true
            }
            Item { Layout.fillWidth: true }

            ToolButton {
                text: "+"
                ToolTip.text: qsTr("Create Entity")
                onClicked: {
                    if (editorEngine) {
                        editorEngine.ecs_bridge().create_entity("New Entity")
                    }
                }
            }
            ToolButton {
                text: "-"
                ToolTip.text: qsTr("Delete Selected")
                onClicked: {}
            }
        }

        TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("Search entities...")
            font.pixelSize: 12
        }

        ListView {
            id: entityList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: ListModel {
                id: entityModel
            }

            delegate: ItemDelegate {
                width: entityList.width
                height: 28

                contentItem: RowLayout {
                    spacing: 6
                    Rectangle {
                        width: 12
                        height: 12
                        radius: 3
                        color: model.active ? "#4CAF50" : "#666"
                    }
                    Text {
                        text: model.name
                        color: palette.text
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                onClicked: {
                    entityList.currentIndex = index
                }
            }

            Component.onCompleted: {
                if (editorEngine) {
                    var entities = editorEngine.ecs_bridge().get_all_entities()
                    for (var i = 0; i < entities.length; i++) {
                        entityModel.append(entities[i])
                    }
                }
            }
        }
    }
}
