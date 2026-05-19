#include <okn/editor/panels/panel_manager.hpp>
#include <algorithm>

namespace okn::editor {

struct PanelManagerImpl {
    std::vector<PanelInfo> panels;
    PanelManager::PanelCallback visibility_callback;
};

PanelManager::PanelManager() : pImpl_(new PanelManagerImpl()) {}
PanelManager::~PanelManager() { delete static_cast<PanelManagerImpl*>(pImpl_); }

auto PanelManager::register_panel(const std::string& id, const std::string& title) -> void {
    auto* impl = static_cast<PanelManagerImpl*>(pImpl_);
    for (auto& p : impl->panels) {
        if (p.id == id) {
            p.title = title;
            return;
        }
    }
    impl->panels.push_back({id, title, PanelVisibility::visible});
}

auto PanelManager::show_panel(const std::string& id) -> void {
    auto* impl = static_cast<PanelManagerImpl*>(pImpl_);
    for (auto& p : impl->panels) {
        if (p.id == id) {
            p.visibility = PanelVisibility::visible;
            if (impl->visibility_callback) impl->visibility_callback(id, true);
            return;
        }
    }
}

auto PanelManager::hide_panel(const std::string& id) -> void {
    auto* impl = static_cast<PanelManagerImpl*>(pImpl_);
    for (auto& p : impl->panels) {
        if (p.id == id) {
            p.visibility = PanelVisibility::hidden;
            if (impl->visibility_callback) impl->visibility_callback(id, false);
            return;
        }
    }
}

auto PanelManager::toggle_panel(const std::string& id) -> void {
    if (is_visible(id)) {
        hide_panel(id);
    } else {
        show_panel(id);
    }
}

auto PanelManager::is_visible(const std::string& id) const -> bool {
    auto* impl = static_cast<const PanelManagerImpl*>(pImpl_);
    for (const auto& p : impl->panels) {
        if (p.id == id) return p.visibility == PanelVisibility::visible;
    }
    return false;
}

auto PanelManager::panel_ids() const -> std::vector<std::string> {
    auto* impl = static_cast<const PanelManagerImpl*>(pImpl_);
    std::vector<std::string> ids;
    for (const auto& p : impl->panels) ids.push_back(p.id);
    return ids;
}

auto PanelManager::panel_info(const std::string& id) const -> PanelInfo {
    auto* impl = static_cast<const PanelManagerImpl*>(pImpl_);
    for (const auto& p : impl->panels) {
        if (p.id == id) return p;
    }
    return {};
}

auto PanelManager::all_panels() const -> std::vector<PanelInfo> {
    return static_cast<const PanelManagerImpl*>(pImpl_)->panels;
}

auto PanelManager::set_visibility_callback(PanelCallback cb) -> void {
    static_cast<PanelManagerImpl*>(pImpl_)->visibility_callback = std::move(cb);
}

} // namespace okn::editor
