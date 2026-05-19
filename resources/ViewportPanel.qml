import QtQuick
import QtQuick.Controls

Item {
    id: viewportPanel
    property var editorEngine

    Rectangle {
        anchors.fill: parent
        color: "#1a1a2e"
        border.color: "#333"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 4

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: qsTr("Viewport")
                    color: "#ccc"
                    font.pixelSize: 13
                }
                Item { Layout.fillWidth: true }

                ComboBox {
                    id: displayMode
                    model: [qsTr("Lit"), qsTr("Wireframe"), qsTr("Unlit"), qsTr("Normals"), qsTr("Depth")]
                    currentIndex: 0
                }
                ComboBox {
                    id: resolutionScale
                    model: ["100%", "75%", "50%", "25%"]
                    currentIndex: 0
                }
                Text {
                    text: qsTr("Draw Calls: 0")
                    color: "#aaa"
                    font.pixelSize: 11
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#0d0d1a"
                border.color: "#444"

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Viewport\n(Click to focus)")
                    color: "#555"
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                    onWheel: function(wheel) { /* Camera zoom */ }
                }
            }
        }
    }
}
