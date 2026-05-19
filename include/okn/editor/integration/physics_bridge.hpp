#pragma once

#include <string>

namespace okn::editor {

struct PhysicsStats {
    uint32_t rigid_bodies = 0;
    uint32_t constraints = 0;
    uint32_t collision_pairs = 0;
    double step_time_ms = 0.0;
};

class PhysicsBridge {
public:
    PhysicsBridge();
    ~PhysicsBridge();

    auto initialize() -> bool;
    auto shutdown() -> void;

    [[nodiscard]] auto get_stats() const -> PhysicsStats;
    auto set_enabled(bool enabled) -> void;
    auto set_gravity(double x, double y, double z) -> void;
    auto simulate_step() -> void;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
