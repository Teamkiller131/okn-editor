#include <okn/editor/integration/script_bridge.hpp>
#include <algorithm>

namespace okn::editor {

struct ScriptBridgeImpl {
    std::vector<ScriptInfo> scripts;
    uint64_t next_script_id = 1;
};

ScriptBridge::ScriptBridge() : pImpl_(new ScriptBridgeImpl()) {}
ScriptBridge::~ScriptBridge() { delete static_cast<ScriptBridgeImpl*>(pImpl_); }

auto ScriptBridge::initialize() -> bool { return true; }
auto ScriptBridge::shutdown() -> void {
    static_cast<ScriptBridgeImpl*>(pImpl_)->scripts.clear();
}

auto ScriptBridge::get_all_scripts() const -> std::vector<ScriptInfo> {
    return static_cast<const ScriptBridgeImpl*>(pImpl_)->scripts;
}

auto ScriptBridge::load_script(uint64_t id) -> bool {
    auto* impl = static_cast<ScriptBridgeImpl*>(pImpl_);
    for (auto& s : impl->scripts) {
        if (s.id == id) { s.is_loaded = true; return true; }
    }
    return false;
}

auto ScriptBridge::unload_script(uint64_t id) -> void {
    auto* impl = static_cast<ScriptBridgeImpl*>(pImpl_);
    for (auto& s : impl->scripts) {
        if (s.id == id) { s.is_loaded = false; return; }
    }
}

auto ScriptBridge::hot_reload(uint64_t id) -> bool { return load_script(id); }

auto ScriptBridge::reload_all() -> void {}

} // namespace okn::editor
