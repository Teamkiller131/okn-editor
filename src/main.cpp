#include <okn/editor/editor.hpp>

#include <cstdlib>
#include <cstring>

// Entry point. `--frames N` runs N frames then exits (headless CI smoke run);
// otherwise the editor runs until the window is closed.
auto main(int argc, char* argv[]) -> int {
    int frames = 0;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            frames = std::atoi(argv[++i]);
        }
    }
    return okn::editor::run_editor(frames);
}
