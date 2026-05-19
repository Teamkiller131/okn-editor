#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace okn::editor {

struct AssetEntry {
    uint64_t id = 0;
    std::string name;
    std::string path;
    std::string type;
    uint64_t size_bytes = 0;
    bool loaded = false;
};

class AssetBridge {
public:
    AssetBridge();
    ~AssetBridge();

    auto initialize() -> bool;
    auto shutdown() -> void;

    [[nodiscard]] auto get_all_assets() const -> std::vector<AssetEntry>;
    [[nodiscard]] auto get_assets_by_type(const std::string& type) const -> std::vector<AssetEntry>;
    [[nodiscard]] auto get_asset(uint64_t id) const -> AssetEntry;
    auto import_asset(const std::string& path) -> uint64_t;
    auto remove_asset(uint64_t id) -> bool;
    auto reload_asset(uint64_t id) -> bool;
    [[nodiscard]] auto asset_count() const -> size_t;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
