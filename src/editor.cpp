#include <okn/editor/editor.hpp>

#include <unigui/unigui.h>
#include <imgui.h>
#include <imgui_internal.h>   // DockBuilder

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>

#if defined(OKN_EDITOR_HAS_SCENE)
#include <okn/render/slice/lua_slice.hpp>
#include <okn/render/slice/slice_components.hpp>
#include <okn/render/sprite2d/sprite_batch.hpp>
#include <okn/math/algebra/quat.hpp>
#include <okn/physics/physics_types.hpp>
#include <okn/physics/dynamics/body.hpp>
#include <okn/ecs/scripting/scripting_bridge.hpp>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>
#endif

namespace okn::editor {
namespace {

#if defined(OKN_EDITOR_HAS_SCENE)
using okn::render::slice::LuaSlice;
using okn::render::slice::SliceWorld;
using okn::render::slice::Transform2D;
using okn::render::slice::SpriteComp;
using okn::render::slice::BodyComp;
using okn::render::slice::PlayerTag;
using okn::ecs::Entity;
using okn::math::Vec2;

const char* kDefaultScene = R"LUA(
set_gravity(20)
spawn_ground(0, -30, 600, 30, 60, 140, 70)
spawn_ground(90, 0, 60, 12, 90, 110, 90)
spawn_player(-40, 40, 16, 16, 240, 240, 255)
spawn_box(30, 60, 14, 14, 220, 120, 60)
spawn_box(34, 90, 12, 12, 200, 90, 150)
land_count = 0
function on_land() land_count = land_count + 1 end
function on_update(dt) end
)LUA";

// Serialize the live scene back to a Lua scene script. The entity kind is recovered
// from the PlayerTag and the physics body type (static -> ground, dynamic -> box).
std::string scene_to_lua(LuaSlice& slice) {
    auto& world = slice.world();
    auto& ecs = world.ecs();
    auto& phys = world.physics();
    std::string out = "-- Authored by the OmniKillerNexus editor.\nset_gravity(20)\n";
    for (auto [e, t] : ecs.query<Transform2D>()) {
        int r = 255, g = 255, b = 255;
        if (auto* s = ecs.get_component<SpriteComp>(e)) {
            r = s->color.r; g = s->color.g; b = s->color.b;
        }
        const char* fn = "spawn_box";
        if (ecs.has_component<PlayerTag>(e)) {
            fn = "spawn_player";
        } else if (auto* bc = ecs.get_component<BodyComp>(e)) {
            if (auto* rb = phys.get_body(bc->body_id)) {
                if (rb->type == okn::physics::BodyType::kStatic) { fn = "spawn_ground"; }
            }
        }
        char line[192];
        std::snprintf(line, sizeof(line), "%s(%.1f, %.1f, %.1f, %.1f, %d, %d, %d)\n",
                      fn, static_cast<double>(t->position.x), static_cast<double>(t->position.y),
                      static_cast<double>(t->size.x), static_cast<double>(t->size.y), r, g, b);
        out += line;
    }
    out += "land_count = 0\nfunction on_land() land_count = land_count + 1 end\n";
    out += "function on_update(dt) end\n";
    return out;
}

enum class UndoKind { Edit, Recreate, Delete };

struct UndoEntry {
    UndoKind kind = UndoKind::Edit;
    Entity entity{};
    Transform2D xform{};
    SpriteComp sprite{};
    bool has_xform = false;
    bool has_sprite = false;
    bool dynamic = false;     // structural spawn: dynamic vs static body
    bool is_player = false;   // structural spawn: re-add the player tag
};

struct EditorState {
    std::unique_ptr<LuaSlice> slice;
    // Name-based reflection over the live World (rebuilt with the slice) — drives the
    // Inspector's GENERIC component listing, so new registered components appear with
    // zero editor code. Typed widgets above it stay the editing fast path.
    std::unique_ptr<okn::ecs::ScriptingBridge> bridge;
    Entity selected{};
    bool has_selection = false;
    bool playing = false;
    bool step_once = false;
    bool dragging = false;
    Vec2 grab_offset{0.0f, 0.0f};
    std::vector<UndoEntry> undo_stack;   // multi-level: each holds a pre-edit transform
    std::vector<UndoEntry> redo_stack;
    std::string status = "no scene";
    int entity_count = 0;
    Vec2 cam_center{0.0f, -5.0f};
    float view_width = 560.0f;
};
EditorState g_state;

bool same_entity(Entity a, Entity b) {
    return a.index() == b.index() && a.generation() == b.generation();
}

void select_player() {
    g_state.selected = g_state.slice->world().player();
    g_state.has_selection = g_state.selected.is_valid();
}

void load_scene() {
    g_state.bridge.reset();   // the old bridge points at the World being replaced
    g_state.slice = std::make_unique<LuaSlice>();
    if (g_state.slice->load_file("slice_scene.lua")) {
        g_state.status = "loaded slice_scene.lua";
    } else {
        g_state.slice->load_string(kDefaultScene);
        g_state.status = "loaded embedded default scene";
    }
    using FieldDesc = okn::ecs::ScriptingBridge::FieldDesc;
    using FT = FieldDesc::Type;
    g_state.bridge = std::make_unique<okn::ecs::ScriptingBridge>(g_state.slice->world().ecs());
    g_state.bridge->register_component<Transform2D>(
        "Transform2D",
        {FieldDesc{"pos.x", offsetof(Transform2D, position) + 0, FT::kF32},
         FieldDesc{"pos.y", offsetof(Transform2D, position) + sizeof(float), FT::kF32},
         FieldDesc{"rotation", offsetof(Transform2D, rotation), FT::kF32},
         FieldDesc{"size.x", offsetof(Transform2D, size) + 0, FT::kF32},
         FieldDesc{"size.y", offsetof(Transform2D, size) + sizeof(float), FT::kF32}});
    g_state.bridge->register_component<SpriteComp>(
        "SpriteComp",
        {FieldDesc{"r", offsetof(SpriteComp, color) + 0, FT::kU8},
         FieldDesc{"g", offsetof(SpriteComp, color) + 1, FT::kU8},
         FieldDesc{"b", offsetof(SpriteComp, color) + 2, FT::kU8},
         FieldDesc{"a", offsetof(SpriteComp, color) + 3, FT::kU8},
         FieldDesc{"texture_id", offsetof(SpriteComp, texture_id), FT::kU32}});
    g_state.bridge->register_component<BodyComp>(
        "BodyComp", {FieldDesc{"body_id", offsetof(BodyComp, body_id), FT::kU32}});
    g_state.bridge->register_component<PlayerTag>(
        "PlayerTag", {FieldDesc{"active", offsetof(PlayerTag, active), FT::kBool}});
    select_player();
    g_state.undo_stack.clear();
    g_state.redo_stack.clear();
}

void save_scene() {
    if (!g_state.slice) { return; }
    std::ofstream f("slice_scene.lua");
    if (f) {
        f << scene_to_lua(*g_state.slice);
        g_state.status = "saved slice_scene.lua";
    } else {
        g_state.status = "save FAILED";
    }
}

// Sync an entity's transform into its physics body (and freeze velocity for drags).
void push_transform_to_body(Entity e, bool freeze_velocity) {
    auto& world = g_state.slice->world();
    auto* t = world.ecs().get_component<Transform2D>(e);
    auto* b = world.ecs().get_component<BodyComp>(e);
    if (t == nullptr || b == nullptr || b->body_id == 0) { return; }
    const okn::math::Quat q = okn::math::Quat::from_axis_angle({0.0f, 0.0f, 1.0f}, t->rotation);
    world.physics().set_transform(b->body_id, {t->position.x, t->position.y, 0.0f}, q);
    if (freeze_velocity) {
        world.physics().set_linear_velocity(b->body_id, {0.0f, 0.0f, 0.0f});
    }
}

// Capture an entity's editable component state for undo (transform + sprite).
UndoEntry snapshot_entity(Entity e) {
    UndoEntry s;
    s.entity = e;
    auto& w = g_state.slice->world().ecs();
    if (auto* t = w.get_component<Transform2D>(e)) { s.xform = *t; s.has_xform = true; }
    if (auto* sp = w.get_component<SpriteComp>(e)) { s.sprite = *sp; s.has_sprite = true; }
    return s;
}

// Restore a captured snapshot onto the live entity.
void apply_snapshot(const UndoEntry& s) {
    auto& w = g_state.slice->world().ecs();
    if (s.has_xform) {
        if (auto* t = w.get_component<Transform2D>(s.entity)) { *t = s.xform; }
        push_transform_to_body(s.entity, true);
    }
    if (s.has_sprite) {
        if (auto* sp = w.get_component<SpriteComp>(s.entity)) { *sp = s.sprite; }
    }
}

// Capture an entity's full spawn parameters (components + body type) so it can be
// recreated for structural undo.
UndoEntry capture_spawn(Entity e) {
    UndoEntry u = snapshot_entity(e);   // xform + sprite
    auto& world = g_state.slice->world();
    if (auto* bc = world.ecs().get_component<BodyComp>(e)) {
        if (auto* b = world.physics().get_body(bc->body_id)) {
            u.dynamic = (b->type == okn::physics::BodyType::kDynamic);
        }
    }
    u.is_player = world.ecs().has_component<PlayerTag>(e);
    return u;
}

// Recreate an entity (with a physics body) from captured spawn params.
Entity editor_spawn(const UndoEntry& p) {
    const Vec2 center = p.has_xform ? p.xform.position : Vec2{0.0f, 0.0f};
    const Vec2 size = p.has_xform ? p.xform.size : Vec2{8.0f, 8.0f};
    return g_state.slice->world().spawn_box(center, size, p.sprite.color, p.dynamic, p.is_player);
}

// Snapshot an entity BEFORE it is edited; a fresh edit invalidates the redo stack
// (the classic undo/redo contract). Covers transform AND sprite edits.
void record_undo(Entity e) {
    if (!g_state.slice) { return; }
    UndoEntry s = snapshot_entity(e);
    if (s.has_xform || s.has_sprite) {
        g_state.undo_stack.push_back(s);
        g_state.redo_stack.clear();
    }
}

// Pop one entry from `from`, apply it to the live scene, and push its inverse onto
// `to`. Handles property edits and structural (recreate/delete) ops uniformly.
bool restore_from(std::vector<UndoEntry>& from, std::vector<UndoEntry>& to,
                  const char* label) {
    if (from.empty() || !g_state.slice) { return false; }
    const UndoEntry entry = from.back();
    from.pop_back();

    if (entry.kind == UndoKind::Edit) {
        to.push_back(snapshot_entity(entry.entity));   // current state -> the other stack
        apply_snapshot(entry);
        g_state.selected = entry.entity;
        g_state.has_selection = true;
    } else if (entry.kind == UndoKind::Recreate) {
        UndoEntry inv = entry;                          // inverse of recreate is delete
        inv.entity = editor_spawn(entry);
        inv.kind = UndoKind::Delete;
        to.push_back(inv);
        g_state.selected = inv.entity;
        g_state.has_selection = true;
    } else {  // UndoKind::Delete
        g_state.slice->world().despawn(entry.entity);
        UndoEntry inv = entry;                          // inverse of delete is recreate
        inv.kind = UndoKind::Recreate;
        to.push_back(inv);
        g_state.has_selection = false;
    }
    g_state.status = label;
    return true;
}

void apply_undo() { restore_from(g_state.undo_stack, g_state.redo_stack, "undo"); }
void apply_redo() { restore_from(g_state.redo_stack, g_state.undo_stack, "redo"); }

// Delete the selected entity (undoable: its inverse recreates it).
void delete_selected() {
    if (!g_state.has_selection || !g_state.slice) { return; }
    UndoEntry u = capture_spawn(g_state.selected);
    u.kind = UndoKind::Recreate;            // to undo a delete, recreate it
    g_state.slice->world().despawn(g_state.selected);
    g_state.undo_stack.push_back(u);
    g_state.redo_stack.clear();
    g_state.has_selection = false;
    g_state.status = "deleted entity";
}

// Duplicate the selected entity, offset so it's visible (undoable: inverse deletes).
void duplicate_selected() {
    if (!g_state.has_selection || !g_state.slice) { return; }
    UndoEntry p = capture_spawn(g_state.selected);
    p.is_player = false;                    // never clone the player tag
    if (p.has_xform) { p.xform.position.x += 8.0f; p.xform.position.y += 8.0f; }
    UndoEntry u = p;
    u.entity = editor_spawn(p);
    u.kind = UndoKind::Delete;              // to undo a create, delete it
    g_state.undo_stack.push_back(u);
    g_state.redo_stack.clear();
    g_state.selected = u.entity;
    g_state.has_selection = true;
    g_state.status = "duplicated entity";
}

void tick_scene() {
    if (!g_state.slice) { return; }
    if (g_state.slice->check_hot_reload()) {
        g_state.has_selection = false;
        g_state.status = "hot-reloaded slice_scene.lua";
        select_player();
    }
    if (g_state.playing || g_state.step_once) {
        float dt = ImGui::GetIO().DeltaTime;
        dt = std::clamp(dt, 0.0f, 1.0f / 30.0f);
        g_state.slice->update(dt);
        g_state.step_once = false;
    }
}

void draw_hierarchy() {
    ImGui::Begin("Hierarchy");
    int count = 0;
    if (g_state.slice) {
        auto& w = g_state.slice->world().ecs();
        for (auto [e, t] : w.query<Transform2D>()) {
            (void)t;
            ++count;
            const bool is_player = w.has_component<PlayerTag>(e);
            char label[64];
            std::snprintf(label, sizeof(label), "%s Entity %u", is_player ? "[player]" : "       ",
                          e.index());
            const bool sel = g_state.has_selection && same_entity(g_state.selected, e);
            if (ImGui::Selectable(label, sel)) {
                g_state.selected = e;
                g_state.has_selection = true;
            }
        }
    }
    g_state.entity_count = count;
    ImGui::End();
}

auto to_u8(float v) -> std::uint8_t {
    return static_cast<std::uint8_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
}

void draw_inspector() {
    ImGui::Begin("Inspector");
    if (g_state.has_selection && g_state.slice) {
        auto& w = g_state.slice->world().ecs();
        auto* t = w.get_component<Transform2D>(g_state.selected);
        auto* s = w.get_component<SpriteComp>(g_state.selected);
        auto* b = w.get_component<BodyComp>(g_state.selected);

        ImGui::Text("Entity %u (gen %u)", g_state.selected.index(), g_state.selected.generation());
        ImGui::Separator();

        bool xform_changed = false;
        if (t != nullptr) {
            ImGui::SeparatorText("Transform2D");
            if (ImGui::DragFloat2("position", &t->position.x, 0.5f)) { xform_changed = true; }
            if (ImGui::IsItemActivated()) { record_undo(g_state.selected); }  // capture pre-edit
            if (ImGui::DragFloat("rotation", &t->rotation, 0.01f)) { xform_changed = true; }
            if (ImGui::IsItemActivated()) { record_undo(g_state.selected); }
            ImGui::DragFloat2("size", &t->size.x, 0.25f, 0.1f, 4000.0f);
            if (ImGui::IsItemActivated()) { record_undo(g_state.selected); }  // size is undoable too
            ImGui::SameLine();
            ImGui::TextDisabled("(visual)");
        }
        if (s != nullptr) {
            ImGui::SeparatorText("Sprite");
            float col[4] = {static_cast<float>(s->color.r) / 255.0f,
                            static_cast<float>(s->color.g) / 255.0f,
                            static_cast<float>(s->color.b) / 255.0f,
                            static_cast<float>(s->color.a) / 255.0f};
            if (ImGui::ColorEdit4("color", col, ImGuiColorEditFlags_NoInputs)) {
                s->color = {to_u8(col[0]), to_u8(col[1]), to_u8(col[2]), to_u8(col[3])};
            }
            if (ImGui::IsItemActivated()) { record_undo(g_state.selected); }  // capture pre-edit color
            ImGui::Text("texture_id %u", s->texture_id);
        }
        if (b != nullptr) {
            ImGui::SeparatorText("Body");
            ImGui::Text("body_id    %u", b->body_id);
        }
        if (xform_changed) { push_transform_to_body(g_state.selected, false); }

        // Generic reflection view: every component the ScriptingBridge knows, by name —
        // components without a typed section above still show up here (read-only bytes;
        // labeled editing needs the per-field descriptor layer, ROADMAP §12B).
        if (g_state.bridge) {
            ImGui::SeparatorText("All components (reflection)");
            std::map<std::string, okn::ecs::ScriptingBridge::ComponentDesc> sorted(
                g_state.bridge->descriptors().begin(), g_state.bridge->descriptors().end());
            for (const auto& [name, desc] : sorted) {
                if (!g_state.bridge->has_component(g_state.selected, name.c_str())) { continue; }
                const auto* bytes = static_cast<const std::uint8_t*>(
                    static_cast<const okn::ecs::ScriptingBridge&>(*g_state.bridge)
                        .component_data(g_state.selected, name.c_str()));
                if (ImGui::TreeNode(name.c_str(), "%s  (%zu bytes)", name.c_str(),
                                    static_cast<std::size_t>(desc.size))) {
                    if (bytes != nullptr && !desc.fields.empty()) {
                        // Labeled, typed values via the per-field descriptors.
                        using FT = okn::ecs::ScriptingBridge::FieldDesc::Type;
                        for (const auto& fd : desc.fields) {
                            const auto* p = bytes + fd.offset;
                            switch (fd.type) {
                                case FT::kF32:
                                    ImGui::Text("%-10s %.3f", fd.name.c_str(),
                                                *reinterpret_cast<const float*>(p));
                                    break;
                                case FT::kF64:
                                    ImGui::Text("%-10s %.3f", fd.name.c_str(),
                                                *reinterpret_cast<const double*>(p));
                                    break;
                                case FT::kI32:
                                    ImGui::Text("%-10s %d", fd.name.c_str(),
                                                *reinterpret_cast<const std::int32_t*>(p));
                                    break;
                                case FT::kU32:
                                    ImGui::Text("%-10s %u", fd.name.c_str(),
                                                *reinterpret_cast<const std::uint32_t*>(p));
                                    break;
                                case FT::kU8:
                                    ImGui::Text("%-10s %u", fd.name.c_str(),
                                                static_cast<unsigned>(*p));
                                    break;
                                case FT::kBool:
                                    ImGui::Text("%-10s %s", fd.name.c_str(),
                                                *reinterpret_cast<const bool*>(p) ? "true" : "false");
                                    break;
                            }
                        }
                    } else if (bytes != nullptr && desc.size > 0) {
                        // Descriptor-less components fall back to a byte preview.
                        const std::size_t n = std::min<std::size_t>(desc.size, 32);
                        std::string hex;
                        hex.reserve(n * 3);
                        for (std::size_t i = 0; i < n; ++i) {
                            char buf[4];
                            std::snprintf(buf, sizeof(buf), "%02X ", bytes[i]);
                            hex += buf;
                        }
                        if (desc.size > n) { hex += "..."; }
                        ImGui::TextDisabled("%s", hex.c_str());
                    } else {
                        ImGui::TextDisabled("(empty / tag component)");
                    }
                    ImGui::TreePop();
                }
            }
        }
    } else {
        ImGui::TextDisabled("(select an entity in the Hierarchy or Viewport)");
    }
    ImGui::End();
}

void draw_viewport() {
    ImGui::Begin("Viewport");

    ImGui::Checkbox("Play", &g_state.playing);
    ImGui::SameLine();
    if (ImGui::Button("Step")) { g_state.step_once = true; }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) { load_scene(); }
    ImGui::SameLine();
    if (ImGui::Button("Save")) { save_scene(); }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(140.0f);
    ImGui::SliderFloat("zoom", &g_state.view_width, 120.0f, 1200.0f, "%.0f u");

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    avail.x = std::max(avail.x, 16.0f);
    avail.y = std::max(avail.y, 16.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p1{p0.x + avail.x, p0.y + avail.y};
    dl->AddRectFilled(p0, p1, IM_COL32(20, 22, 32, 255));
    dl->PushClipRect(p0, p1, true);

    const float scale = avail.x / g_state.view_width;
    const Vec2 c = g_state.cam_center;
    auto to_screen = [&](Vec2 wp) -> ImVec2 {
        return ImVec2(p0.x + avail.x * 0.5f + (wp.x - c.x) * scale,
                      p0.y + avail.y * 0.5f - (wp.y - c.y) * scale);
    };
    auto to_world = [&](ImVec2 sp) -> Vec2 {
        return Vec2{c.x + (sp.x - (p0.x + avail.x * 0.5f)) / scale,
                    c.y - (sp.y - (p0.y + avail.y * 0.5f)) / scale};
    };

    // World grid + brighter origin axes, so positions are readable while editing.
    {
        constexpr float step = 20.0f;
        auto absf = [](float v) { return v < 0.0f ? -v : v; };
        const float half_w = g_state.view_width * 0.5f;
        const float half_h = half_w * (avail.y / avail.x);
        const int xi0 = static_cast<int>((c.x - half_w) / step) - 1;
        const int xi1 = static_cast<int>((c.x + half_w) / step) + 1;
        for (int xi = xi0; xi <= xi1; ++xi) {
            const float x = static_cast<float>(xi) * step;
            const bool axis = absf(x) < 0.01f;
            dl->AddLine(to_screen({x, c.y - half_h}), to_screen({x, c.y + half_h}),
                        axis ? IM_COL32(92, 94, 124, 200) : IM_COL32(44, 46, 58, 150),
                        axis ? 1.5f : 1.0f);
        }
        const int yi0 = static_cast<int>((c.y - half_h) / step) - 1;
        const int yi1 = static_cast<int>((c.y + half_h) / step) + 1;
        for (int yi = yi0; yi <= yi1; ++yi) {
            const float y = static_cast<float>(yi) * step;
            const bool axis = absf(y) < 0.01f;
            dl->AddLine(to_screen({c.x - half_w, y}), to_screen({c.x + half_w, y}),
                        axis ? IM_COL32(92, 94, 124, 200) : IM_COL32(44, 46, 58, 150),
                        axis ? 1.5f : 1.0f);
        }
    }

    if (g_state.slice) {
        auto& w = g_state.slice->world().ecs();
        const auto groups = g_state.slice->world().build_sprites().build();
        for (const auto& gr : groups) {
            for (std::size_t v = 0; v + 4 <= gr.vertices.size(); v += 4) {
                const auto& a = gr.vertices[v + 0];
                const auto& b = gr.vertices[v + 1];
                const auto& d = gr.vertices[v + 2];
                const auto& e = gr.vertices[v + 3];
                const ImU32 col = IM_COL32(a.color.r, a.color.g, a.color.b, a.color.a);
                dl->AddQuadFilled(to_screen(a.pos), to_screen(b.pos),
                                  to_screen(d.pos), to_screen(e.pos), col);
            }
        }
        if (g_state.has_selection) {
            if (auto* t = w.get_component<Transform2D>(g_state.selected)) {
                const float hw = t->size.x * 0.5f, hh = t->size.y * 0.5f;
                const ImVec2 a = to_screen({t->position.x - hw, t->position.y - hh});
                const ImVec2 b = to_screen({t->position.x + hw, t->position.y + hh});
                dl->AddRect(ImVec2(std::min(a.x, b.x), std::min(a.y, b.y)),
                            ImVec2(std::max(a.x, b.x), std::max(a.y, b.y)),
                            IM_COL32(255, 220, 60, 255), 0.0f, 0, 2.0f);
            }
        }
    }
    dl->PopClipRect();

    // Mouse interaction: click to pick, drag to move (a gizmo).
    ImGui::SetCursorScreenPos(p0);
    ImGui::InvisibleButton("##viewport_canvas", avail, ImGuiButtonFlags_MouseButtonLeft);
    if (g_state.slice && ImGui::IsItemHovered()) {
        auto& w = g_state.slice->world().ecs();
        const Vec2 wm = to_world(ImGui::GetIO().MousePos);
        if (ImGui::IsMouseClicked(0)) {
            Entity pick{};
            bool found = false;
            for (auto [e, t] : w.query<Transform2D>()) {
                const float hw = t->size.x * 0.5f, hh = t->size.y * 0.5f;
                if (wm.x >= t->position.x - hw && wm.x <= t->position.x + hw &&
                    wm.y >= t->position.y - hh && wm.y <= t->position.y + hh) {
                    pick = e;  // last hit = topmost (drawn later)
                    found = true;
                }
            }
            if (found) {
                g_state.selected = pick;
                g_state.has_selection = true;
                g_state.dragging = true;
                record_undo(pick);
                if (auto* t = w.get_component<Transform2D>(pick)) {
                    g_state.grab_offset = Vec2{wm.x - t->position.x, wm.y - t->position.y};
                }
            } else {
                g_state.has_selection = false;
            }
        }
    }
    if (g_state.dragging && g_state.has_selection && ImGui::IsMouseDown(0)) {
        auto& w = g_state.slice->world().ecs();
        if (auto* t = w.get_component<Transform2D>(g_state.selected)) {
            const Vec2 wm = to_world(ImGui::GetIO().MousePos);
            t->position = Vec2{wm.x - g_state.grab_offset.x, wm.y - g_state.grab_offset.y};
            push_transform_to_body(g_state.selected, true);
        }
    }
    if (!ImGui::IsMouseDown(0)) { g_state.dragging = false; }

    ImGui::End();
}

void draw_assets() {
    ImGui::Begin("Assets");
    namespace fs = std::filesystem;
    std::error_code ec;
    bool any = false;
    for (const auto& entry : fs::directory_iterator(fs::current_path(), ec)) {
        if (entry.path().extension() == ".lua") {
            any = true;
            const std::string name = entry.path().filename().string();
            if (ImGui::Selectable(name.c_str())) {
                if (g_state.slice && g_state.slice->load_file(name)) {
                    g_state.status = "loaded " + name;
                    select_player();
                }
            }
        }
    }
    if (!any) { ImGui::TextDisabled("(no .lua scenes found)"); }
    ImGui::End();
}

void draw_console() {
    ImGui::Begin("Console");
    ImGui::Text("OmniKillerNexus Editor — P4 (gizmos, save, assets, undo).");
    ImGui::Text("scene: %s   entities: %d", g_state.status.c_str(), g_state.entity_count);
    if (g_state.slice) {
        ImGui::Text("state: %s   landings: %.0f   |  drag in viewport to move; Ctrl+Z undo",
                    g_state.playing ? "PLAYING" : "paused",
                    g_state.slice->get_number("land_count"));
    }
    ImGui::End();
}
#endif  // OKN_EDITOR_HAS_SCENE

void build_default_layout(ImGuiID dock_id, const ImVec2& size) {
    ImGui::DockBuilderRemoveNode(dock_id);
    ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dock_id, size);

    ImGuiID center = dock_id;
    const ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.18f, nullptr, &center);
    const ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.24f, nullptr, &center);
    const ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.26f, nullptr, &center);

    ImGui::DockBuilderDockWindow("Hierarchy", left);
    ImGui::DockBuilderDockWindow("Inspector", right);
    ImGui::DockBuilderDockWindow("Console", bottom);
    ImGui::DockBuilderDockWindow("Assets", bottom);
    ImGui::DockBuilderDockWindow("Viewport", center);
    ImGui::DockBuilderFinish(dock_id);
}

void draw_dock_host() {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    const ImGuiWindowFlags host_flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("##okn_editor_host", nullptr, host_flags);
    ImGui::PopStyleVar(3);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Scene")) {
#if defined(OKN_EDITOR_HAS_SCENE)
                save_scene();
#endif
            }
            if (ImGui::MenuItem("Reload Scene")) {
#if defined(OKN_EDITOR_HAS_SCENE)
                load_scene();
#endif
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
#if defined(OKN_EDITOR_HAS_SCENE)
                apply_undo();
#endif
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
#if defined(OKN_EDITOR_HAS_SCENE)
                apply_redo();
#endif
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
#if defined(OKN_EDITOR_HAS_SCENE)
                duplicate_selected();
#endif
            }
            if (ImGui::MenuItem("Delete", "Del")) {
#if defined(OKN_EDITOR_HAS_SCENE)
                delete_selected();
#endif
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Hierarchy");
            ImGui::MenuItem("Inspector");
            ImGui::MenuItem("Viewport");
            ImGui::MenuItem("Console");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    const ImGuiID dock_id = ImGui::GetID("okn_editor_dockspace");
    static bool layout_built = false;
    if (!layout_built) {
        layout_built = true;
        build_default_layout(dock_id, vp->WorkSize);
    }
    ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
}

void draw_frame() {
#if defined(OKN_EDITOR_HAS_SCENE)
    tick_scene();
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) { apply_undo(); }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y)) { apply_redo(); }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_D)) { duplicate_selected(); }
    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) { delete_selected(); }
#endif
    draw_dock_host();

#if defined(OKN_EDITOR_HAS_SCENE)
    draw_hierarchy();
    draw_inspector();
    draw_viewport();
    draw_assets();
    draw_console();
#else
    ImGui::Begin("Hierarchy"); ImGui::TextDisabled("(scene support not built)"); ImGui::End();
    ImGui::Begin("Inspector"); ImGui::TextDisabled("(no selection)"); ImGui::End();
    ImGui::Begin("Viewport");  ImGui::TextDisabled("(scene viewport)"); ImGui::End();
    ImGui::Begin("Console");   ImGui::TextUnformatted("OmniKillerNexus Editor — UniGUI shell."); ImGui::End();
#endif
}

}  // namespace

int run_editor(int maxFrames) {
    unigui::AppConfig cfg;
    cfg.title = "OmniKillerNexus Editor";
    cfg.width = 1400;
    cfg.height = 860;

    if (!unigui::Init(cfg)) { return 1; }
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#if defined(OKN_EDITOR_HAS_SCENE)
    load_scene();
#endif
    unigui::Run([] { draw_frame(); }, maxFrames);
    return 0;
}

int run_selftest() {
#if defined(OKN_EDITOR_HAS_SCENE)
    LuaSlice a;
    if (!a.load_file("slice_scene.lua")) { a.load_string(kDefaultScene); }
    const int before = static_cast<int>(a.world().build_sprites().sprite_count());

    const std::string lua = scene_to_lua(a);
    LuaSlice b;
    if (!b.load_string(lua)) {
        std::fprintf(stderr, "selftest: reload of serialized scene FAILED: %s\n",
                     b.last_error().c_str());
        return 1;
    }
    const int after = static_cast<int>(b.world().build_sprites().sprite_count());
    const bool serialize_ok = (before > 0 && before == after);
    std::fprintf(stderr, "selftest: serialize round-trip before=%d after=%d %s\n",
                 before, after, serialize_ok ? "OK" : "FAIL");

    // Multi-level undo/redo on the real editor state, headless: edit an entity
    // twice, undo twice, redo once, and check the transform tracks the stacks.
    bool undo_ok = true;
    load_scene();
    if (g_state.slice && g_state.has_selection) {
        const Entity e = g_state.selected;
        auto pos = [&]() -> float {
            auto* t = g_state.slice->world().ecs().get_component<Transform2D>(e);
            return t ? t->position.x : 0.0f;
        };
        auto move_to = [&](float x) {
            if (auto* t = g_state.slice->world().ecs().get_component<Transform2D>(e)) {
                t->position.x = x;
            }
        };
        auto approx = [](float p, float q) { const float d = p - q; return d < 1e-3f && d > -1e-3f; };

        const float base = pos();
        record_undo(e); move_to(base + 1.0f);     // edit 1 -> base+1
        record_undo(e); move_to(base + 2.0f);     // edit 2 -> base+2
        apply_undo();   const float u1 = pos();   // expect base+1
        apply_undo();   const float u2 = pos();   // expect base
        apply_redo();   const float r1 = pos();   // expect base+1
        undo_ok = approx(u1, base + 1.0f) && approx(u2, base) && approx(r1, base + 1.0f);
        std::fprintf(stderr,
            "selftest: undo/redo base=%.2f undo1=%.2f undo2=%.2f redo1=%.2f %s\n",
            base, u1, u2, r1, undo_ok ? "OK" : "FAIL");

        // Sprite color is now undoable too (the snapshot captures Transform + Sprite).
        if (auto* sp = g_state.slice->world().ecs().get_component<SpriteComp>(e)) {
            const std::uint8_t cr0 = sp->color.r;
            record_undo(e);
            sp->color.r = static_cast<std::uint8_t>(cr0 ^ 0xFFu);   // edit the color
            const std::uint8_t cr1 = sp->color.r;
            apply_undo();
            auto* sp2 = g_state.slice->world().ecs().get_component<SpriteComp>(e);
            const bool color_ok = sp2 != nullptr && sp2->color.r == cr0 && cr1 != cr0;
            std::fprintf(stderr, "selftest: color undo r=%u->%u->%u %s\n",
                         static_cast<unsigned>(cr0), static_cast<unsigned>(cr1),
                         static_cast<unsigned>(sp2 ? sp2->color.r : 0), color_ok ? "OK" : "FAIL");
            undo_ok = undo_ok && color_ok;
        }

        // Structural undo: duplicate + delete change the entity count and are undoable.
        {
            auto count = [&]() -> int {
                return static_cast<int>(g_state.slice->world().ecs().entity_count());
            };
            const int n0 = count();
            duplicate_selected();  const int n1 = count();   // +1
            apply_undo();          const int n2 = count();   // back to n0
            apply_redo();          const int n3 = count();   // +1 again
            delete_selected();     const int n4 = count();   // -1
            apply_undo();          const int n5 = count();   // recreated -> +1
            const bool struct_ok = (n1 == n0 + 1) && (n2 == n0) && (n3 == n0 + 1)
                                && (n4 == n0) && (n5 == n0 + 1);
            std::fprintf(stderr,
                "selftest: struct undo n=%d dup=%d undo=%d redo=%d del=%d undo=%d %s\n",
                n0, n1, n2, n3, n4, n5, struct_ok ? "OK" : "FAIL");
            undo_ok = undo_ok && struct_ok;
        }
    } else {
        std::fprintf(stderr, "selftest: undo/redo SKIPPED (no scene/selection)\n");
    }

    // Generic reflection (the Inspector's "All components" section runs on this):
    // the ScriptingBridge must resolve the live scene by NAME — the player entity
    // reflects its components, component_data matches the typed pointers, and a
    // name-based query sees every transform the typed query sees.
    bool bridge_ok = false;
    if (g_state.slice && g_state.bridge && g_state.slice->world().player().is_valid()) {
        auto& w = g_state.slice->world().ecs();
        const Entity e = g_state.slice->world().player();   // fresh — selection may be
                                                            // stale after the undo tests
        const bool names_ok = g_state.bridge->registered_count() == 4
                           && g_state.bridge->has_component(e, "Transform2D")
                           && g_state.bridge->has_component(e, "PlayerTag");
        const void* raw = static_cast<const okn::ecs::ScriptingBridge&>(*g_state.bridge)
                              .component_data(e, "Transform2D");
        const bool data_ok = raw != nullptr && raw == w.get_component<Transform2D>(e);
        std::size_t typed_n = 0;
        for ([[maybe_unused]] auto [qe, qt] : w.query<Transform2D>()) { ++typed_n; }
        const bool query_ok = g_state.bridge->query("Transform2D").size() == typed_n && typed_n > 0;
        // Per-field reflection: the named field resolves to the typed member's address.
        const void* fx = g_state.bridge->field_data(e, "Transform2D", "pos.x");
        const bool field_ok = fx != nullptr
                           && fx == &w.get_component<Transform2D>(e)->position.x;
        bridge_ok = names_ok && data_ok && query_ok && field_ok;
        std::fprintf(stderr,
            "selftest: reflection names=%d data=%d query=%zu/%zu field=%d %s\n",
            names_ok ? 1 : 0, data_ok ? 1 : 0,
            g_state.bridge->query("Transform2D").size(), typed_n, field_ok ? 1 : 0,
            bridge_ok ? "OK" : "FAIL");
    } else {
        std::fprintf(stderr, "selftest: reflection SKIPPED (no scene/bridge)\n");
    }

    const bool ok = serialize_ok && undo_ok && bridge_ok;
    std::fprintf(stderr, "selftest: %s\n", ok ? "EDITOR SELFTEST OK" : "EDITOR SELFTEST FAIL");
    return ok ? 0 : 1;
#else
    std::fprintf(stderr, "selftest: scene support not built\n");
    return 0;
#endif
}

}  // namespace okn::editor
