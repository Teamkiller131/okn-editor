#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QObject>
#include <QString>
#include <okn/editor/editor_engine.hpp>
#include <okn/editor/project/project_manager.hpp>
#include <okn/editor/command_manager.hpp>
#include <okn/editor/panels/panel_manager.hpp>
#include <okn/editor/integration/ecs_bridge.hpp>
#include <okn/editor/integration/render_bridge.hpp>
#include <okn/editor/integration/asset_bridge.hpp>
#include <okn/editor/editor_types.hpp>

namespace okn::editor {

class EditorEngineAdapter : public QObject {
    Q_OBJECT
public:
    explicit EditorEngineAdapter(QObject* parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE ProjectManager& project_manager() {
        return EditorEngine::instance().project_manager();
    }
    Q_INVOKABLE CommandManager& command_manager() {
        return EditorEngine::instance().command_manager();
    }
    Q_INVOKABLE PanelManager& panel_manager() {
        return EditorEngine::instance().panel_manager();
    }
    Q_INVOKABLE EcsBridge& ecs_bridge() {
        return EditorEngine::instance().ecs_bridge();
    }
    Q_INVOKABLE RenderBridge& render_bridge() {
        return EditorEngine::instance().render_bridge();
    }
    Q_INVOKABLE AssetBridge& asset_bridge() {
        return EditorEngine::instance().asset_bridge();
    }
    Q_INVOKABLE bool save_project() {
        return EditorEngine::instance().save_project();
    }
    Q_INVOKABLE void add_log(const QString& message, int level) {
        EditorEngine::instance().add_log(message.toStdString(),
            static_cast<LogEntry::Level>(level));
    }
    Q_INVOKABLE void clear_logs() {
        EditorEngine::instance().clear_logs();
    }
};

struct QtAppHandle {
    std::unique_ptr<QGuiApplication> app;
    std::unique_ptr<QQmlApplicationEngine> engine;
    EditorEngineAdapter* adapter = nullptr;
};

auto create_qt_app(int argc, char* argv[]) -> void* {
    auto* handle = new QtAppHandle();

    handle->app = std::make_unique<QGuiApplication>(argc, argv);
    handle->app->setApplicationName("OmniKillerNexus Editor");
    handle->app->setOrganizationName("OmniKillerNexus");

    EditorEngine::instance().initialize();

    handle->engine = std::make_unique<QQmlApplicationEngine>();
    handle->adapter = new EditorEngineAdapter(handle->engine.get());
    handle->engine->rootContext()->setContextProperty("editorEngine", handle->adapter);
    handle->engine->load(QUrl(QStringLiteral("qrc:/okn/editor/qml/main.qml")));

    if (handle->engine->rootObjects().isEmpty()) {
        delete handle;
        return nullptr;
    }

    return handle;
}

auto run_qt_app(void* handle_ptr) -> int {
    auto* handle = static_cast<QtAppHandle*>(handle_ptr);
    return handle->app->exec();
}

auto destroy_qt_app(void* handle_ptr) -> void {
    delete static_cast<QtAppHandle*>(handle_ptr);
}

} // namespace okn::editor

#include "editor_app_qt.moc"
