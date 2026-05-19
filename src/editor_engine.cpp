#include <okn/editor/editor_engine.hpp>
#include <okn/editor/config.hpp>
#include <okn/editor/project/project_manager.hpp>
#include <okn/editor/command_manager.hpp>
#include <okn/editor/panels/panel_manager.hpp>
#include <okn/editor/integration/ecs_bridge.hpp>
#include <okn/editor/integration/render_bridge.hpp>
#include <okn/editor/integration/asset_bridge.hpp>
#include <okn/editor/integration/physics_bridge.hpp>
#include <okn/editor/integration/audio_bridge.hpp>
#include <okn/editor/integration/script_bridge.hpp>
#include <okn/editor/integration/network_bridge.hpp>
#include <okn/editor/integration/platform_bridge.hpp>

namespace okn::editor {

struct EditorEngineImpl {
    std::unique_ptr<ProjectManager> project_manager;
    std::unique_ptr<CommandManager> command_manager;
    std::unique_ptr<PanelManager> panel_manager;
    std::unique_ptr<EcsBridge> ecs_bridge;
    std::unique_ptr<RenderBridge> render_bridge;
    std::unique_ptr<AssetBridge> asset_bridge;
    std::unique_ptr<PhysicsBridge> physics_bridge;
    std::unique_ptr<AudioBridge> audio_bridge;
    std::unique_ptr<ScriptBridge> script_bridge;
    std::unique_ptr<NetworkBridge> network_bridge;
    std::unique_ptr<PlatformBridge> platform_bridge;
    EditorSettings settings;
    std::vector<LogEntry> logs;
    bool initialized = false;
};

EditorEngine::EditorEngine() : pImpl_(new EditorEngineImpl()) {}
EditorEngine::~EditorEngine() { delete static_cast<EditorEngineImpl*>(pImpl_); }

auto EditorEngine::instance() -> EditorEngine& {
    static EditorEngine engine;
    return engine;
}

auto EditorEngine::initialize() -> bool {
    auto* impl = static_cast<EditorEngineImpl*>(pImpl_);
    if (impl->initialized) return true;

    impl->project_manager = std::make_unique<ProjectManager>();
    impl->command_manager = std::make_unique<CommandManager>();
    impl->panel_manager = std::make_unique<PanelManager>();
    impl->ecs_bridge = std::make_unique<EcsBridge>();
    impl->render_bridge = std::make_unique<RenderBridge>();
    impl->asset_bridge = std::make_unique<AssetBridge>();
    impl->physics_bridge = std::make_unique<PhysicsBridge>();
    impl->audio_bridge = std::make_unique<AudioBridge>();
    impl->script_bridge = std::make_unique<ScriptBridge>();
    impl->network_bridge = std::make_unique<NetworkBridge>();
    impl->platform_bridge = std::make_unique<PlatformBridge>();

    impl->ecs_bridge->initialize();
    impl->render_bridge->initialize();
    impl->asset_bridge->initialize();
    impl->physics_bridge->initialize();
    impl->audio_bridge->initialize();
    impl->script_bridge->initialize();
    impl->network_bridge->initialize();
    impl->platform_bridge->initialize();

    impl->panel_manager->register_panel("viewport", "Viewport");
    impl->panel_manager->register_panel("hierarchy", "Hierarchy");
    impl->panel_manager->register_panel("inspector", "Inspector");
    impl->panel_manager->register_panel("asset_browser", "Asset Browser");
    impl->panel_manager->register_panel("console", "Console");
    impl->panel_manager->register_panel("profiler", "Profiler");

    impl->initialized = true;
    return true;
}

auto EditorEngine::shutdown() -> void {
    auto* impl = static_cast<EditorEngineImpl*>(pImpl_);
    impl->ecs_bridge->shutdown();
    impl->render_bridge->shutdown();
    impl->asset_bridge->shutdown();
    impl->physics_bridge->shutdown();
    impl->audio_bridge->shutdown();
    impl->script_bridge->shutdown();
    impl->network_bridge->shutdown();
    impl->platform_bridge->shutdown();
    impl->initialized = false;
}

auto EditorEngine::is_initialized() const -> bool {
    return static_cast<const EditorEngineImpl*>(pImpl_)->initialized;
}

auto EditorEngine::new_project(const std::string& path, const std::string& name) -> bool {
    return static_cast<EditorEngineImpl*>(pImpl_)->project_manager->new_project(path, name);
}

auto EditorEngine::open_project(const std::string& path) -> bool {
    return static_cast<EditorEngineImpl*>(pImpl_)->project_manager->open_project(path);
}

auto EditorEngine::save_project() -> bool {
    return static_cast<EditorEngineImpl*>(pImpl_)->project_manager->save_project();
}

auto EditorEngine::close_project() -> void {
    static_cast<EditorEngineImpl*>(pImpl_)->project_manager->close_project();
}

auto EditorEngine::project_manager() -> ProjectManager& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->project_manager;
}

auto EditorEngine::command_manager() -> CommandManager& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->command_manager;
}

auto EditorEngine::panel_manager() -> PanelManager& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->panel_manager;
}

auto EditorEngine::ecs_bridge() -> EcsBridge& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->ecs_bridge;
}

auto EditorEngine::render_bridge() -> RenderBridge& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->render_bridge;
}

auto EditorEngine::asset_bridge() -> AssetBridge& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->asset_bridge;
}

auto EditorEngine::physics_bridge() -> PhysicsBridge& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->physics_bridge;
}

auto EditorEngine::audio_bridge() -> AudioBridge& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->audio_bridge;
}

auto EditorEngine::script_bridge() -> ScriptBridge& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->script_bridge;
}

auto EditorEngine::network_bridge() -> NetworkBridge& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->network_bridge;
}

auto EditorEngine::platform_bridge() -> PlatformBridge& {
    return *static_cast<EditorEngineImpl*>(pImpl_)->platform_bridge;
}

auto EditorEngine::settings() -> EditorSettings& {
    return static_cast<EditorEngineImpl*>(pImpl_)->settings;
}

auto EditorEngine::save_settings() -> void {}

auto EditorEngine::load_settings() -> void {}

auto EditorEngine::add_log(const std::string& message, LogEntry::Level level) -> void {
    auto* impl = static_cast<EditorEngineImpl*>(pImpl_);
    impl->logs.push_back({0, message, level});
    if (impl->logs.size() > kConsoleMaxLines) {
        impl->logs.erase(impl->logs.begin());
    }
}

auto EditorEngine::get_logs() const -> std::vector<LogEntry> {
    return static_cast<const EditorEngineImpl*>(pImpl_)->logs;
}

auto EditorEngine::clear_logs() -> void {
    static_cast<EditorEngineImpl*>(pImpl_)->logs.clear();
}

} // namespace okn::editor
