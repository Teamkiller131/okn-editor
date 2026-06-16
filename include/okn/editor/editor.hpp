#pragma once

// OmniKillerNexus editor — a Dear ImGui editor built on TeamkillerUniGUI.
// No third-party (unigui/imgui) headers here; the shell lives in editor.cpp.

namespace okn::editor {

// Run the editor. maxFrames > 0 stops after N frames (headless CI smoke / screenshots);
// 0 runs until the window is closed. Returns 0 on success, 1 if init failed.
int run_editor(int maxFrames = 0);

// Headless: load the scene, serialize it back to Lua, reload that, and verify the
// entity count round-trips. No window. Returns 0 on success, 1 on failure.
int run_selftest();

} // namespace okn::editor
