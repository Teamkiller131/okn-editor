#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace okn::editor {

using PanelId = std::string;
using EntityId = uint64_t;
using AssetId = uint64_t;
using CommandId = uint64_t;

enum class PanelVisibility : uint8_t {
    hidden,
    visible,
    floating
};

enum class EditorTheme : uint8_t {
    light,
    dark,
    system
};

struct EditorSettings {
    EditorTheme theme = EditorTheme::dark;
    std::string language = "en";
    bool show_fps = true;
    bool auto_save = true;
    int auto_save_interval_sec = 300;
    std::string recent_project_path;
};

struct LogEntry {
    int64_t timestamp;
    std::string message;
    enum class Level : uint8_t { debug, info, warning, error, fatal } level;
};

} // namespace okn::editor
