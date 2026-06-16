// okn-editor stub: built when TeamkillerUniGUI (unigui::unigui) is unavailable, so
// the okn-editor library target and its SDK references still resolve. The real
// editor (src/editor.cpp) builds inside the OmniKillerNexus root, where unigui is
// added from third_party/.

namespace okn::editor {
int okn_editor_stub_symbol = 0;  // avoids an empty translation unit / empty archive
}
