#include <okn/editor/integration/asset_bridge.hpp>
#include <algorithm>

namespace okn::editor {

struct AssetBridgeImpl {
    std::vector<AssetEntry> assets;
    uint64_t next_asset_id = 1;
};

AssetBridge::AssetBridge() : pImpl_(new AssetBridgeImpl()) {}
AssetBridge::~AssetBridge() { delete static_cast<AssetBridgeImpl*>(pImpl_); }

auto AssetBridge::initialize() -> bool { return true; }
auto AssetBridge::shutdown() -> void {
    static_cast<AssetBridgeImpl*>(pImpl_)->assets.clear();
}

auto AssetBridge::get_all_assets() const -> std::vector<AssetEntry> {
    return static_cast<const AssetBridgeImpl*>(pImpl_)->assets;
}

auto AssetBridge::get_assets_by_type(const std::string& type) const -> std::vector<AssetEntry> {
    std::vector<AssetEntry> result;
    auto* impl = static_cast<const AssetBridgeImpl*>(pImpl_);
    for (const auto& a : impl->assets) {
        if (a.type == type) result.push_back(a);
    }
    return result;
}

auto AssetBridge::get_asset(uint64_t id) const -> AssetEntry {
    auto* impl = static_cast<const AssetBridgeImpl*>(pImpl_);
    for (const auto& a : impl->assets) {
        if (a.id == id) return a;
    }
    return {};
}

auto AssetBridge::import_asset(const std::string& path) -> uint64_t {
    auto* impl = static_cast<AssetBridgeImpl*>(pImpl_);
    AssetEntry e{};
    e.id = impl->next_asset_id++;
    e.path = path;
    auto slash = path.find_last_of("/\\");
    e.name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    e.loaded = true;
    impl->assets.push_back(e);
    return e.id;
}

auto AssetBridge::remove_asset(uint64_t id) -> bool {
    auto* impl = static_cast<AssetBridgeImpl*>(pImpl_);
    auto it = std::find_if(impl->assets.begin(), impl->assets.end(),
        [id](const auto& a) { return a.id == id; });
    if (it == impl->assets.end()) return false;
    impl->assets.erase(it);
    return true;
}

auto AssetBridge::reload_asset(uint64_t id) -> bool {
    return true;
}

auto AssetBridge::asset_count() const -> size_t {
    return static_cast<const AssetBridgeImpl*>(pImpl_)->assets.size();
}

} // namespace okn::editor
