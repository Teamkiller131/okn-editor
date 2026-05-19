#include <okn/editor/integration/audio_bridge.hpp>
#include <algorithm>

namespace okn::editor {

struct AudioBridgeImpl {
    std::vector<AudioClipInfo> clips;
    uint64_t next_clip_id = 1;
    float master_volume = 1.0f;
};

AudioBridge::AudioBridge() : pImpl_(new AudioBridgeImpl()) {}
AudioBridge::~AudioBridge() { delete static_cast<AudioBridgeImpl*>(pImpl_); }

auto AudioBridge::initialize() -> bool { return true; }
auto AudioBridge::shutdown() -> void {
    static_cast<AudioBridgeImpl*>(pImpl_)->clips.clear();
}

auto AudioBridge::get_all_clips() const -> std::vector<AudioClipInfo> {
    return static_cast<const AudioBridgeImpl*>(pImpl_)->clips;
}

auto AudioBridge::play_clip(uint64_t id) -> void {
    auto* impl = static_cast<AudioBridgeImpl*>(pImpl_);
    for (auto& c : impl->clips) {
        if (c.id == id) { c.is_playing = true; return; }
    }
}

auto AudioBridge::stop_clip(uint64_t id) -> void {
    auto* impl = static_cast<AudioBridgeImpl*>(pImpl_);
    for (auto& c : impl->clips) {
        if (c.id == id) { c.is_playing = false; return; }
    }
}

auto AudioBridge::set_master_volume(float vol) -> void {
    static_cast<AudioBridgeImpl*>(pImpl_)->master_volume = vol;
}

auto AudioBridge::master_volume() const -> float {
    return static_cast<const AudioBridgeImpl*>(pImpl_)->master_volume;
}

} // namespace okn::editor
