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
#endif

namespace okn::editor {
namespace {

#if defined(OKN_EDITOR_HAS_SCENE)
using okn::render::slice::LuaSlice;
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

struct EditorState {
    std::unique_ptr<LuaSlice> slice;
    Entity selected{};
    bool has_selection = false;
    bool playing = false;
    bool step_once = false;
    std::string status = "no scene";
    int entity_count = 0;
    // 2D viewport camera (world units).
    Vec2 cam_center{0.0f, -5.0f};
    float view_width = 560.0f;
};
EditorState g_state;

bool same_entity(Entity a, Entity b) {
    return a.index() == b.index() && a.generation() == b.generation();
}

void load_scene() {
    g_state.slice = std::make_unique<LuaSlice>();
    if (g_state.slice->load_file("slice_scene.lua")) {
        g_state.status = "loaded slice_scene.lua";
    } else {
        g_state.slice->load_string(kDefaultScene);
        g_state.status = "loaded embedded default scene";
    }
    // Select the player by default so the Inspector is populated.
    g_state.selected = g_state.slice->world().player();
    g_state.has_selection = g_state.selected.is_valid();
}

void tick_scene() {
    if (!g_state.slice) { return; }
    // Live content iteration: re-run slice_scene.lua when it changes on disk.
    if (g_state.slice->check_hot_reload()) {
        g_state.has_selection = false;
        g_state.status = "hot-reloaded slice_scene.lua";
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
            xform_changed |= ImGui::DragFloat2("position", &t->position.x, 0.5f);
            xform_changed |= ImGui::DragFloat("rotation", &t->rotation, 0.01f);
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

        // Push transform edits to the physics body so they hold (and survive Play).
        if (xform_changed && t != nullptr && b != nullptr && b->body_id != 0) {
            const okn::math::Quat q = okn::math::Quat::from_axis_angle({0.0f, 0.0f, 1.0f}, t->rotation);
            g_state.slice->world().physics().set_transform(
                b->body_id, {t->position.x, t->position.y, 0.0f}, q);
        }
    } else {
        ImGui::TextDisabled("(select an entity in the Hierarchy)");
    }
    ImGui::End();
}

void draw_viewport() {
    ImGui::Begin("Viewport");

    ImGui::Checkbox("Play", &g_state.playing);
    ImGui::SameLine();
    if (ImGui::Button("Step")) { g_state.step_once = true; }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) { load_scene(); g_state.has_selection = false; }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160.0f);
    ImGui::SliderFloat("zoom", &g_state.view_width, 120.0f, 1200.0f, "%.0f u");

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x < 16.0f) { avail.x = 16.0f; }
    if (avail.y < 16.0f) { avail.y = 16.0f; }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p1{p0.x + avail.x, p0.y + avail.y};
    dl->AddRectFilled(p0, p1, IM_COL32(20, 22, 32, 255));
    dl->PushClipRect(p0, p1, true);

    const float scale = avail.x / g_state.view_width;
    const Vec2 c = g_state.cam_center;
    auto to_screen = [&](Vec2 wp) -> ImVec2 {
        return ImVec2(p0.x + avail.x * 0.5f + (wp.x - c.x) * scale,
                      p0.y + avail.y * 0.5f - (wp.y - c.y) * scale);  // +Y up
    };

    if (g_state.slice) {
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
        // Outline the selected entity.
        if (g_state.has_selection) {
            auto& w = g_state.slice->world().ecs();
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
    ImGui::Dummy(avail);   // reserve the drawn region
    ImGui::End();
}

void draw_console() {
    ImGui::Begin("Console");
    ImGui::Text("OmniKillerNexus Editor — P3 (edit components + hot-reload Lua).");
    ImGui::Text("scene: %s   entities: %d", g_state.status.c_str(), g_state.entity_count);
    if (g_state.slice) {
        ImGui::Text("state: %s   landings: %.0f", g_state.playing ? "PLAYING" : "paused",
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
            ImGui::MenuItem("Open Scene…");
            ImGui::MenuItem("Save");
            ImGui::Separator();
            ImGui::MenuItem("Exit");
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
#endif
    draw_dock_host();

#if defined(OKN_EDITOR_HAS_SCENE)
    draw_hierarchy();
    draw_inspector();
    draw_viewport();
    ImGui::Begin("Assets");
    ImGui::TextDisabled("slice_scene.lua");
    ImGui::End();
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

}  // namespace okn::editor
