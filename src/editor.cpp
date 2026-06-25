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
#include <cstdint>
#include <filesystem>
#include <fstream>
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

struct UndoEntry {
    Entity entity{};
    Transform2D xform{};
    bool valid = false;
};

struct EditorState {
    std::unique_ptr<LuaSlice> slice;
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
    g_state.slice = std::make_unique<LuaSlice>();
    if (g_state.slice->load_file("slice_scene.lua")) {
        g_state.status = "loaded slice_scene.lua";
    } else {
        g_state.slice->load_string(kDefaultScene);
        g_state.status = "loaded embedded default scene";
    }
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

// Snapshot an entity's transform BEFORE it is edited; a fresh edit invalidates the
// redo stack (the classic undo/redo contract).
void record_undo(Entity e) {
    if (auto* t = g_state.slice->world().ecs().get_component<Transform2D>(e)) {
        g_state.undo_stack.push_back(UndoEntry{e, *t, true});
        g_state.redo_stack.clear();
    }
}

// Move the current transform onto `to` and restore the snapshot from `from`.
bool restore_from(std::vector<UndoEntry>& from, std::vector<UndoEntry>& to,
                  const char* label) {
    if (from.empty() || !g_state.slice) { return false; }
    const UndoEntry entry = from.back();
    from.pop_back();
    auto* t = g_state.slice->world().ecs().get_component<Transform2D>(entry.entity);
    if (t == nullptr) { return false; }
    to.push_back(UndoEntry{entry.entity, *t, true});   // current state -> the other stack
    *t = entry.xform;                                  // restore the snapshot
    push_transform_to_body(entry.entity, true);
    g_state.selected = entry.entity;
    g_state.has_selection = true;
    g_state.status = label;
    return true;
}

void apply_undo() { restore_from(g_state.undo_stack, g_state.redo_stack, "undo"); }
void apply_redo() { restore_from(g_state.redo_stack, g_state.undo_stack, "redo"); }

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
            ImGui::Text("texture_id %u", s->texture_id);
        }
        if (b != nullptr) {
            ImGui::SeparatorText("Body");
            ImGui::Text("body_id    %u", b->body_id);
        }
        if (xform_changed) { push_transform_to_body(g_state.selected, false); }
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
    } else {
        std::fprintf(stderr, "selftest: undo/redo SKIPPED (no scene/selection)\n");
    }

    const bool ok = serialize_ok && undo_ok;
    std::fprintf(stderr, "selftest: %s\n", ok ? "EDITOR SELFTEST OK" : "EDITOR SELFTEST FAIL");
    return ok ? 0 : 1;
#else
    std::fprintf(stderr, "selftest: scene support not built\n");
    return 0;
#endif
}

}  // namespace okn::editor
