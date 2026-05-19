#pragma once

#include <cstdint>
#include <string>

namespace okn::editor {

inline constexpr auto kEditorVersion = "1.0.0";
inline constexpr auto kEditorName    = "OmniKillerNexus Editor";
inline constexpr auto kEditorOrg     = "OmniKillerNexus";

inline constexpr uint32_t kMaxUndoSteps = 256;
inline constexpr uint32_t kDefaultViewportFPS = 60;
inline constexpr uint32_t kConsoleMaxLines = 10000;

inline constexpr auto kProjectFileExtension = ".oknproj";

} // namespace okn::editor
