#include <okn/editor/editor.hpp>

#include <unigui/unigui.h>
#include <imgui.h>

#include <cstdio>
#include <memory>
#include <string>

#if defined(OKN_EDITOR_HAS_SCENE)
#include <okn/render/slice/lua_slice.hpp>
#include <okn/render/slice/slice_components.hpp>
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
    std::string status = "no scene";
    int entity_count = 0;
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

void draw_inspector() {
    ImGui::Begin("Inspector");
    if (g_state.has_selection && g_state.slice) {
        auto& w = g_state.slice->world().ecs();
        ImGui::Text("Entity %u (gen %u)", g_state.selected.index(), g_state.selected.generation());
        ImGui::Separator();
        if (auto* t = w.get_component<Transform2D>(g_state.selected)) {
            ImGui::SeparatorText("Transform2D");
            ImGui::Text("position   %8.2f , %8.2f", static_cast<double>(t->position.x),
                        static_cast<double>(t->position.y));
            ImGui::Text("rotation   %8.3f rad", static_cast<double>(t->rotation));
            ImGui::Text("size       %8.2f x %8.2f", static_cast<double>(t->size.x),
                        static_cast<double>(t->size.y));
        }
        if (auto* s = w.get_component<SpriteComp>(g_state.selected)) {
            ImGui::SeparatorText("Sprite");
            float col[4] = {static_cast<float>(s->color.r) / 255.0f,
                            static_cast<float>(s->color.g) / 255.0f,
                            static_cast<float>(s->color.b) / 255.0f,
                            static_cast<float>(s->color.a) / 255.0f};
            ImGui::ColorEdit4("color", col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            ImGui::Text("texture_id %u", s->texture_id);
        }
        if (auto* b = w.get_component<BodyComp>(g_state.selected)) {
            ImGui::SeparatorText("Body");
            ImGui::Text("body_id    %u", b->body_id);
        }
    } else {
        ImGui::TextDisabled("(select an entity in the Hierarchy)");
    }
    ImGui::End();
}
#endif  // OKN_EDITOR_HAS_SCENE

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
    ImGui::DockSpace(ImGui::GetID("okn_editor_dockspace"), ImVec2(0.0f, 0.0f),
                     ImGuiDockNodeFlags_None);
    ImGui::End();
}

void draw_frame() {
    draw_dock_host();

#if defined(OKN_EDITOR_HAS_SCENE)
    draw_hierarchy();
    draw_inspector();

    ImGui::Begin("Viewport");
    ImGui::TextDisabled("(software-rendered scene viewport — P2)");
    ImGui::End();

    ImGui::Begin("Assets");
    ImGui::TextDisabled("slice_scene.lua");
    ImGui::End();

    ImGui::Begin("Console");
    ImGui::Text("OmniKillerNexus Editor — P1 (live ECS bridge).");
    ImGui::Text("scene: %s", g_state.status.c_str());
    ImGui::Text("entities: %d", g_state.entity_count);
    ImGui::End();
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
