#include <okn/editor/integration/render_bridge.hpp>

namespace okn::editor {

struct RenderBridgeImpl {
    bool ready = false;
    uint32_t vp_width = 1280;
    uint32_t vp_height = 720;
    bool wireframe = false;
    bool show_grid = true;
    RenderStats stats;
};

RenderBridge::RenderBridge() : pImpl_(new RenderBridgeImpl()) {}
RenderBridge::~RenderBridge() { delete static_cast<RenderBridgeImpl*>(pImpl_); }

auto RenderBridge::initialize() -> bool {
    auto* impl = static_cast<RenderBridgeImpl*>(pImpl_);
    impl->ready = true;
    return true;
}

auto RenderBridge::shutdown() -> void {
    static_cast<RenderBridgeImpl*>(pImpl_)->ready = false;
}

auto RenderBridge::is_ready() const -> bool {
    return static_cast<const RenderBridgeImpl*>(pImpl_)->ready;
}

auto RenderBridge::get_stats() const -> RenderStats {
    return static_cast<const RenderBridgeImpl*>(pImpl_)->stats;
}

auto RenderBridge::resize_viewport(uint32_t width, uint32_t height) -> void {
    auto* impl = static_cast<RenderBridgeImpl*>(pImpl_);
    impl->vp_width = width;
    impl->vp_height = height;
}

auto RenderBridge::get_viewport_handle() const -> void* { return nullptr; }

auto RenderBridge::viewport_width() const -> uint32_t {
    return static_cast<const RenderBridgeImpl*>(pImpl_)->vp_width;
}

auto RenderBridge::viewport_height() const -> uint32_t {
    return static_cast<const RenderBridgeImpl*>(pImpl_)->vp_height;
}

auto RenderBridge::set_wireframe(bool enable) -> void {
    static_cast<RenderBridgeImpl*>(pImpl_)->wireframe = enable;
}

auto RenderBridge::set_show_grid(bool enable) -> void {
    static_cast<RenderBridgeImpl*>(pImpl_)->show_grid = enable;
}

} // namespace okn::editor
