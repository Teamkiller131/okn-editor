#pragma once

#include <string>
#include <vector>

namespace okn::editor {

struct ScriptInfo {
    uint64_t id = 0;
    std::string name;
    std::string path;
    std::string language;
    bool is_loaded = false;
};

class ScriptBridge {
public:
    ScriptBridge();
    ~ScriptBridge();

    auto initialize() -> bool;
    auto shutdown() -> void;

    [[nodiscard]] auto get_all_scripts() const -> std::vector<ScriptInfo>;
    auto load_script(uint64_t id) -> bool;
    auto unload_script(uint64_t id) -> void;
    auto hot_reload(uint64_t id) -> bool;
    auto reload_all() -> void;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
