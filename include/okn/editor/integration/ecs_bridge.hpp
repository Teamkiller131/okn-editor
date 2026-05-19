#pragma once

#include <string>
#include <vector>
#include <memory>

namespace okn::editor {

struct EntityInfo {
    uint64_t id = 0;
    std::string name;
    bool active = true;
    uint64_t parent_id = 0;
    std::vector<std::string> component_types;
};

class EcsBridge {
public:
    EcsBridge();
    ~EcsBridge();

    auto initialize() -> bool;
    auto shutdown() -> void;

    [[nodiscard]] auto entity_count() const -> size_t;
    [[nodiscard]] auto get_all_entities() const -> std::vector<EntityInfo>;
    [[nodiscard]] auto get_entity(uint64_t id) const -> EntityInfo;
    auto create_entity(const std::string& name) -> uint64_t;
    auto destroy_entity(uint64_t id) -> bool;
    auto set_entity_name(uint64_t id, const std::string& name) -> void;
    auto set_entity_parent(uint64_t id, uint64_t parent_id) -> void;
    auto add_component(uint64_t id, const std::string& type) -> bool;
    auto remove_component(uint64_t id, const std::string& type) -> bool;
    [[nodiscard]] auto get_component_data(uint64_t id, const std::string& type) const -> std::string;
    auto set_component_data(uint64_t id, const std::string& type, const std::string& json) -> void;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
