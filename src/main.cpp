#include <okn/editor/editor_app.hpp>

auto main(int argc, char* argv[]) -> int {
    okn::editor::EditorApp app{argc, argv};
    return app.run();
}
