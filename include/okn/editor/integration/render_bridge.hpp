#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace okn::editor {

struct RenderStats {
    uint32_t draw_calls = 0;
    uint32_t triangles = 0;
    uint32_t vertices = 0;
    uint32_t shader_switches = 0;
    double frame_time_ms = 0.0;
};

class RenderBridge {
public:
    RenderBridge();
    ~RenderBridge();

    auto initialize() -> bool;
    auto shutdown() -> void;

    [[nodiscard]] auto is_ready() const -> bool;
    [[nodiscard]] auto get_stats() const -> RenderStats;
    auto resize_viewport(uint32_t width, uint32_t height) -> void;
    [[nodiscard]] auto get_viewport_handle() const -> void*;

    [[nodiscard]] auto viewport_width() const -> uint32_t;
    [[nodiscard]] auto viewport_height() const -> uint32_t;

    auto set_wireframe(bool enable) -> void;
    auto set_show_grid(bool enable) -> void;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
