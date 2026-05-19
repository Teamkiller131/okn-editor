#pragma once

#include <string>
#include <vector>
#include <functional>
#include <okn/editor/editor_types.hpp>

namespace okn::editor {

struct PanelInfo {
    std::string id;
    std::string title;
    PanelVisibility visibility = PanelVisibility::visible;
};

class PanelManager {
public:
    PanelManager();
    ~PanelManager();

    auto register_panel(const std::string& id, const std::string& title) -> void;
    auto show_panel(const std::string& id) -> void;
    auto hide_panel(const std::string& id) -> void;
    auto toggle_panel(const std::string& id) -> void;
    [[nodiscard]] auto is_visible(const std::string& id) const -> bool;
    [[nodiscard]] auto panel_ids() const -> std::vector<std::string>;
    [[nodiscard]] auto panel_info(const std::string& id) const -> PanelInfo;
    [[nodiscard]] auto all_panels() const -> std::vector<PanelInfo>;

    using PanelCallback = std::function<void(const std::string&, bool)>;
    auto set_visibility_callback(PanelCallback cb) -> void;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
