#pragma once

#include <string>
#include <vector>

namespace okn::editor {

struct AudioClipInfo {
    uint64_t id = 0;
    std::string name;
    std::string path;
    double duration_sec = 0.0;
    bool is_playing = false;
};

class AudioBridge {
public:
    AudioBridge();
    ~AudioBridge();

    auto initialize() -> bool;
    auto shutdown() -> void;

    [[nodiscard]] auto get_all_clips() const -> std::vector<AudioClipInfo>;
    auto play_clip(uint64_t id) -> void;
    auto stop_clip(uint64_t id) -> void;
    auto set_master_volume(float vol) -> void;
    [[nodiscard]] auto master_volume() const -> float;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
