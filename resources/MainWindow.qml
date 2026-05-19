import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: mainWindow
    property var editorEngine

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        Rectangle {
            SplitView.preferredWidth: 300
            SplitView.minimumWidth: 150
            color: palette.window
            border.color: palette.mid

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 0

                TabBar {
                    id: leftTabs
                    Layout.fillWidth: true
                    TabButton { text: qsTr("Hierarchy") }
                    TabButton { text: qsTr("Profiler") }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: leftTabs.currentIndex

                    HierarchyPanel {
                        editorEngine: mainWindow.editorEngine
                    }
                    ProfilerPanel {
                        editorEngine: mainWindow.editorEngine
                    }
                }
            }
        }

        Rectangle {
            SplitView.fillWidth: true
            color: "#1e1e1e"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                ViewportPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    editorEngine: mainWindow.editorEngine
                }
            }
        }

        Rectangle {
            SplitView.preferredWidth: 320
            SplitView.minimumWidth: 200
            color: palette.window
            border.color: palette.mid

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 0

                TabBar {
                    id: rightTabs
                    Layout.fillWidth: true
                    TabButton { text: qsTr("Inspector") }
                    TabButton { text: qsTr("Assets") }
                    TabButton { text: qsTr("Console") }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: rightTabs.currentIndex

                    InspectorPanel {
                        editorEngine: mainWindow.editorEngine
                    }
                    AssetBrowserPanel {
                        editorEngine: mainWindow.editorEngine
                    }
                    ConsolePanel {
                        editorEngine: mainWindow.editorEngine
                    }
                }
            }
        }
    }
}
