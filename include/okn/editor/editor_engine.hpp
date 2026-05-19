#pragma once

#include <string>
#include <vector>
#include <memory>
#include <okn/editor/editor_types.hpp>

namespace okn::editor {

class ProjectManager;
class CommandManager;
class PanelManager;
class EcsBridge;
class RenderBridge;
class AssetBridge;
class PhysicsBridge;
class AudioBridge;
class ScriptBridge;
class NetworkBridge;
class PlatformBridge;

class EditorEngine {
public:
    [[nodiscard]] static auto instance() -> EditorEngine&;

    auto initialize() -> bool;
    auto shutdown() -> void;
    [[nodiscard]] auto is_initialized() const -> bool;

    auto new_project(const std::string& path, const std::string& name) -> bool;
    auto open_project(const std::string& path) -> bool;
    auto save_project() -> bool;
    auto close_project() -> void;

    [[nodiscard]] auto project_manager() -> ProjectManager&;
    [[nodiscard]] auto command_manager() -> CommandManager&;
    [[nodiscard]] auto panel_manager() -> PanelManager&;
    [[nodiscard]] auto ecs_bridge() -> EcsBridge&;
    [[nodiscard]] auto render_bridge() -> RenderBridge&;
    [[nodiscard]] auto asset_bridge() -> AssetBridge&;
    [[nodiscard]] auto physics_bridge() -> PhysicsBridge&;
    [[nodiscard]] auto audio_bridge() -> AudioBridge&;
    [[nodiscard]] auto script_bridge() -> ScriptBridge&;
    [[nodiscard]] auto network_bridge() -> NetworkBridge&;
    [[nodiscard]] auto platform_bridge() -> PlatformBridge&;

    [[nodiscard]] auto settings() -> EditorSettings&;
    auto save_settings() -> void;
    auto load_settings() -> void;

    auto add_log(const std::string& message, LogEntry::Level level = LogEntry::Level::info) -> void;
    [[nodiscard]] auto get_logs() const -> std::vector<LogEntry>;
    auto clear_logs() -> void;

    EditorEngine(const EditorEngine&) = delete;
    auto operator=(const EditorEngine&) -> EditorEngine& = delete;
    EditorEngine(EditorEngine&&) = delete;
    auto operator=(EditorEngine&&) -> EditorEngine& = delete;

private:
    EditorEngine();
    ~EditorEngine();

    void* pImpl_ = nullptr;
};

} // namespace okn::editor
