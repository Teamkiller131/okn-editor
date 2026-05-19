#pragma once

#include <string>
#include <cstdint>

namespace okn::editor {

struct NetworkStats {
    uint64_t bytes_sent = 0;
    uint64_t bytes_received = 0;
    uint32_t packets_lost = 0;
    uint32_t latency_ms = 0;
    uint32_t connections = 0;
};

class NetworkBridge {
public:
    NetworkBridge();
    ~NetworkBridge();

    auto initialize() -> bool;
    auto shutdown() -> void;

    [[nodiscard]] auto get_stats() const -> NetworkStats;
    auto set_enabled(bool enabled) -> void;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
