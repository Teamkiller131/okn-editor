#include <okn/editor/integration/ecs_bridge.hpp>

namespace okn::editor {

struct EcsBridgeImpl {
    std::vector<EntityInfo> entities;
    uint64_t next_entity_id = 1;
};

EcsBridge::EcsBridge() : pImpl_(new EcsBridgeImpl()) {}
EcsBridge::~EcsBridge() { delete static_cast<EcsBridgeImpl*>(pImpl_); }

auto EcsBridge::initialize() -> bool { return true; }
auto EcsBridge::shutdown() -> void {
    auto* impl = static_cast<EcsBridgeImpl*>(pImpl_);
    impl->entities.clear();
}

auto EcsBridge::entity_count() const -> size_t {
    return static_cast<const EcsBridgeImpl*>(pImpl_)->entities.size();
}

auto EcsBridge::get_all_entities() const -> std::vector<EntityInfo> {
    return static_cast<const EcsBridgeImpl*>(pImpl_)->entities;
}

auto EcsBridge::get_entity(uint64_t id) const -> EntityInfo {
    auto* impl = static_cast<const EcsBridgeImpl*>(pImpl_);
    for (const auto& e : impl->entities) {
        if (e.id == id) return e;
    }
    return {};
}

auto EcsBridge::create_entity(const std::string& name) -> uint64_t {
    auto* impl = static_cast<EcsBridgeImpl*>(pImpl_);
    EntityInfo info{};
    info.id = impl->next_entity_id++;
    info.name = name;
    info.active = true;
    impl->entities.push_back(info);
    return info.id;
}

auto EcsBridge::destroy_entity(uint64_t id) -> bool {
    auto* impl = static_cast<EcsBridgeImpl*>(pImpl_);
    auto it = std::find_if(impl->entities.begin(), impl->entities.end(),
        [id](const auto& e) { return e.id == id; });
    if (it == impl->entities.end()) return false;
    impl->entities.erase(it);
    return true;
}

auto EcsBridge::set_entity_name(uint64_t id, const std::string& name) -> void {
    auto* impl = static_cast<EcsBridgeImpl*>(pImpl_);
    for (auto& e : impl->entities) {
        if (e.id == id) { e.name = name; return; }
    }
}

auto EcsBridge::set_entity_parent(uint64_t id, uint64_t parent_id) -> void {
    auto* impl = static_cast<EcsBridgeImpl*>(pImpl_);
    for (auto& e : impl->entities) {
        if (e.id == id) { e.parent_id = parent_id; return; }
    }
}

auto EcsBridge::add_component(uint64_t id, const std::string& type) -> bool {
    auto* impl = static_cast<EcsBridgeImpl*>(pImpl_);
    for (auto& e : impl->entities) {
        if (e.id == id) {
            for (const auto& c : e.component_types) {
                if (c == type) return false;
            }
            e.component_types.push_back(type);
            return true;
        }
    }
    return false;
}

auto EcsBridge::remove_component(uint64_t id, const std::string& type) -> bool {
    auto* impl = static_cast<EcsBridgeImpl*>(pImpl_);
    for (auto& e : impl->entities) {
        if (e.id == id) {
            auto it = std::find(e.component_types.begin(), e.component_types.end(), type);
            if (it == e.component_types.end()) return false;
            e.component_types.erase(it);
            return true;
        }
    }
    return false;
}

auto EcsBridge::get_component_data(uint64_t id, const std::string& type) const -> std::string {
    return "{}";
}

auto EcsBridge::set_component_data(uint64_t id, const std::string& type, const std::string& json) -> void {}

} // namespace okn::editor
