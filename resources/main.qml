import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: appWindow
    visible: true
    width: 1920
    height: 1080
    title: "OmniKillerNexus Editor"

    property var editorEngine

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Action {
                text: qsTr("&New Project")
                shortcut: StandardKey.New
                onTriggered: console.log("New Project")
            }
            Action {
                text: qsTr("&Open Project")
                shortcut: StandardKey.Open
                onTriggered: console.log("Open Project")
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Save")
                shortcut: StandardKey.Save
                onTriggered: {
                    if (editorEngine) editorEngine.save_project()
                }
            }
            MenuSeparator {}
            Action {
                text: qsTr("E&xit")
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: qsTr("&Edit")
            Action {
                text: qsTr("&Undo")
                shortcut: StandardKey.Undo
                onTriggered: {
                    if (editorEngine) editorEngine.command_manager().undo()
                }
            }
            Action {
                text: qsTr("&Redo")
                shortcut: StandardKey.Redo
                onTriggered: {
                    if (editorEngine) editorEngine.command_manager().redo()
                }
            }
        }
        Menu {
            title: qsTr("&View")
            Action {
                text: qsTr("&Viewport")
                checkable: true
                checked: true
                onTriggered: {
                    if (editorEngine) editorEngine.panel_manager().toggle_panel("viewport")
                }
            }
            Action {
                text: qsTr("&Hierarchy")
                checkable: true
                checked: true
                onTriggered: {
                    if (editorEngine) editorEngine.panel_manager().toggle_panel("hierarchy")
                }
            }
            Action {
                text: qsTr("&Inspector")
                checkable: true
                checked: true
                onTriggered: {
                    if (editorEngine) editorEngine.panel_manager().toggle_panel("inspector")
                }
            }
            Action {
                text: qsTr("Asset &Browser")
                checkable: true
                checked: true
                onTriggered: {
                    if (editorEngine) editorEngine.panel_manager().toggle_panel("asset_browser")
                }
            }
            Action {
                text: qsTr("&Console")
                checkable: true
                checked: true
                onTriggered: {
                    if (editorEngine) editorEngine.panel_manager().toggle_panel("console")
                }
            }
            Action {
                text: qsTr("&Profiler")
                checkable: true
                checked: false
                onTriggered: {
                    if (editorEngine) editorEngine.panel_manager().toggle_panel("profiler")
                }
            }
        }
        Menu {
            title: qsTr("&Help")
            Action {
                text: qsTr("&About")
                onTriggered: console.log("About OmniKillerNexus Editor")
            }
        }
    }

    header: ToolBar {
        id: toolbar
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("▶")
                ToolTip.text: qsTr("Play")
                onClicked: console.log("Play")
            }
            ToolButton {
                text: qsTr("⏸")
                ToolTip.text: qsTr("Pause")
                onClicked: console.log("Pause")
            }
            ToolButton {
                text: qsTr("⏹")
                ToolTip.text: qsTr("Stop")
                onClicked: console.log("Stop")
            }
            ToolSeparator {}

            ToolButton {
                text: qsTr("↶")
                ToolTip.text: qsTr("Undo")
                enabled: editorEngine ? editorEngine.command_manager().can_undo() : false
                onClicked: {
                    if (editorEngine) editorEngine.command_manager().undo()
                }
            }
            ToolButton {
                text: qsTr("↷")
                ToolTip.text: qsTr("Redo")
                enabled: editorEngine ? editorEngine.command_manager().can_redo() : false
                onClicked: {
                    if (editorEngine) editorEngine.command_manager().redo()
                }
            }
            Item { Layout.fillWidth: true }
        }
    }

    MainWindow {
        anchors.fill: parent
        editorEngine: appWindow.editorEngine
    }

    footer: StatusBar {
        id: statusBar
        RowLayout {
            anchors.fill: parent
            Label {
                text: editorEngine && editorEngine.project_manager().is_open()
                      ? editorEngine.project_manager().project_name()
                      : qsTr("No project open")
            }
            Item { Layout.fillWidth: true }
            Label {
                text: qsTr("FPS: 60")
            }
        }
    }

    Component.onCompleted: {
        if (editorEngine) {
            editorEngine.add_log("Editor started", 0)
        }
    }
}
