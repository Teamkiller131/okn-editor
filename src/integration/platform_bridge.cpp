#include <okn/editor/integration/platform_bridge.hpp>

namespace okn::editor {

struct PlatformBridgeImpl {
    PlatformInfo info;
};

PlatformBridge::PlatformBridge() : pImpl_(new PlatformBridgeImpl()) {}
PlatformBridge::~PlatformBridge() { delete static_cast<PlatformBridgeImpl*>(pImpl_); }

auto PlatformBridge::initialize() -> bool {
    auto* impl = static_cast<PlatformBridgeImpl*>(pImpl_);
#ifdef _WIN32
    impl->info.os_name = "Windows";
#elif defined(__linux__)
    impl->info.os_name = "Linux";
#elif defined(__APPLE__)
    impl->info.os_name = "macOS";
#else
    impl->info.os_name = "Unknown";
#endif
    return true;
}

auto PlatformBridge::shutdown() -> void {}

auto PlatformBridge::get_info() const -> PlatformInfo {
    return static_cast<const PlatformBridgeImpl*>(pImpl_)->info;
}

auto PlatformBridge::get_working_dir() const -> std::string { return "."; }

auto PlatformBridge::open_url(const std::string& url) -> void {}

} // namespace okn::editor
