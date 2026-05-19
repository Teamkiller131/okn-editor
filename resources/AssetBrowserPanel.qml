import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: assetBrowserPanel
    property var editorEngine

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: qsTr("Asset Browser")
                color: palette.text
                font.pixelSize: 13
                font.bold: true
            }
            Item { Layout.fillWidth: true }

            ToolButton {
                text: qsTr("Import")
                onClicked: {}
            }
        }

        RowLayout {
            Layout.fillWidth: true
            TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Filter assets...")
                font.pixelSize: 12
            }
            ComboBox {
                model: [qsTr("All"), qsTr("Model"), qsTr("Texture"), qsTr("Material"), qsTr("Audio"), qsTr("Script")]
                currentIndex: 0
            }
        }

        ListView {
            id: assetList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: ListModel {
                id: assetModel
            }

            delegate: ItemDelegate {
                width: assetList.width
                height: 32

                contentItem: RowLayout {
                    spacing: 8
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 3
                        color: "#607D8B"
                        Text {
                            anchors.centerIn: parent
                            text: model.type.charAt(0).toUpperCase()
                            color: "white"
                            font.pixelSize: 10
                        }
                    }
                    ColumnLayout {
                        spacing: 0
                        Layout.fillWidth: true
                        Text {
                            text: model.name
                            color: palette.text
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                        Text {
                            text: model.type + " · " + (model.size_bytes / 1024).toFixed(1) + " KB"
                            color: "#888"
                            font.pixelSize: 10
                        }
                    }
                }

                onClicked: {
                    assetList.currentIndex = index
                }
            }

            Component.onCompleted: {
                if (editorEngine) {
                    var assets = editorEngine.asset_bridge().get_all_assets()
                    for (var i = 0; i < assets.length; i++) {
                        assetModel.append(assets[i])
                    }
                }
            }
        }
    }
}
