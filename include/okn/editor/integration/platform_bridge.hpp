#pragma once

#include <string>
#include <cstdint>

namespace okn::editor {

struct PlatformInfo {
    std::string os_name;
    std::string cpu_brand;
    uint32_t core_count = 0;
    uint64_t total_ram_mb = 0;
    uint64_t available_ram_mb = 0;
    std::string gpu_name;
    uint64_t gpu_vram_mb = 0;
};

class PlatformBridge {
public:
    PlatformBridge();
    ~PlatformBridge();

    auto initialize() -> bool;
    auto shutdown() -> void;

    [[nodiscard]] auto get_info() const -> PlatformInfo;
    [[nodiscard]] auto get_working_dir() const -> std::string;
    auto open_url(const std::string& url) -> void;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
