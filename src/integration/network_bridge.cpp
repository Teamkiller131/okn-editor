#include <okn/editor/integration/network_bridge.hpp>

namespace okn::editor {

struct NetworkBridgeImpl {
    bool enabled = false;
    NetworkStats stats;
};

NetworkBridge::NetworkBridge() : pImpl_(new NetworkBridgeImpl()) {}
NetworkBridge::~NetworkBridge() { delete static_cast<NetworkBridgeImpl*>(pImpl_); }

auto NetworkBridge::initialize() -> bool { return true; }
auto NetworkBridge::shutdown() -> void {}

auto NetworkBridge::get_stats() const -> NetworkStats {
    return static_cast<const NetworkBridgeImpl*>(pImpl_)->stats;
}

auto NetworkBridge::set_enabled(bool enabled) -> void {
    static_cast<NetworkBridgeImpl*>(pImpl_)->enabled = enabled;
}

} // namespace okn::editor
