#include <okn/editor/integration/physics_bridge.hpp>

namespace okn::editor {

struct PhysicsBridgeImpl {
    bool enabled = false;
    double gravity[3] = {0.0, -9.81, 0.0};
    PhysicsStats stats;
};

PhysicsBridge::PhysicsBridge() : pImpl_(new PhysicsBridgeImpl()) {}
PhysicsBridge::~PhysicsBridge() { delete static_cast<PhysicsBridgeImpl*>(pImpl_); }

auto PhysicsBridge::initialize() -> bool { return true; }
auto PhysicsBridge::shutdown() -> void {}

auto PhysicsBridge::get_stats() const -> PhysicsStats {
    return static_cast<const PhysicsBridgeImpl*>(pImpl_)->stats;
}

auto PhysicsBridge::set_enabled(bool enabled) -> void {
    static_cast<PhysicsBridgeImpl*>(pImpl_)->enabled = enabled;
}

auto PhysicsBridge::set_gravity(double x, double y, double z) -> void {
    auto* impl = static_cast<PhysicsBridgeImpl*>(pImpl_);
    impl->gravity[0] = x;
    impl->gravity[1] = y;
    impl->gravity[2] = z;
}

auto PhysicsBridge::simulate_step() -> void {}

} // namespace okn::editor
