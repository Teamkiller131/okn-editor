#include <okn/editor/editor.hpp>

#include <unigui/unigui.h>
#include <imgui.h>

namespace okn::editor {
namespace {

// A full-viewport DockSpace host + the editor's empty panels. P0 establishes the
// shell; P1-P3 fill the panels with live ECS/physics/render bridges.
void draw_frame() {
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
            ImGui::MenuItem("Assets");
            ImGui::MenuItem("Console");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::DockSpace(ImGui::GetID("okn_editor_dockspace"), ImVec2(0.0f, 0.0f),
                     ImGuiDockNodeFlags_None);
    ImGui::End();

    ImGui::Begin("Hierarchy");
    ImGui::TextDisabled("(no scene loaded)");
    ImGui::End();

    ImGui::Begin("Inspector");
    ImGui::TextDisabled("(no selection)");
    ImGui::End();

    ImGui::Begin("Viewport");
    ImGui::TextDisabled("(scene viewport)");
    ImGui::End();

    ImGui::Begin("Assets");
    ImGui::TextDisabled("(asset browser)");
    ImGui::End();

    ImGui::Begin("Console");
    ImGui::TextUnformatted("OmniKillerNexus Editor — UniGUI shell (P0).");
    ImGui::TextDisabled("ECS / physics / render bridges land in P1–P3.");
    ImGui::End();
}

} // namespace

int run_editor(int maxFrames) {
    unigui::AppConfig cfg;
    cfg.title = "OmniKillerNexus Editor";
    cfg.width = 1400;
    cfg.height = 860;

    // Manual Init/Run: ImGui requires DockingEnable to be set BEFORE the first
    // NewFrame(), which RunApp()'s callback runs too late for. Run() shuts down
    // automatically when the loop ends.
    if (!unigui::Init(cfg)) { return 1; }
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    unigui::Run([] { draw_frame(); }, maxFrames);
    return 0;
}

} // namespace okn::editor
