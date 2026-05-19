#include <okn/editor/editor_app.hpp>

#ifdef OKN_HAS_QT
namespace okn::editor {
    auto create_qt_app(int argc, char* argv[]) -> void*;
    auto run_qt_app(void* handle) -> int;
    auto destroy_qt_app(void* handle) -> void;
}
#endif

namespace okn::editor {

struct EditorAppImpl {
    int argc = 0;
    char** argv = nullptr;
#ifdef OKN_HAS_QT
    void* qt_handle = nullptr;
#endif
};

EditorApp::EditorApp(int argc, char* argv[]) : pImpl_(new EditorAppImpl()) {
    auto* impl = static_cast<EditorAppImpl*>(pImpl_);
    impl->argc = argc;
    impl->argv = argv;
}

EditorApp::~EditorApp() {
#ifdef OKN_HAS_QT
    auto* impl = static_cast<EditorAppImpl*>(pImpl_);
    if (impl->qt_handle) {
        destroy_qt_app(impl->qt_handle);
    }
#endif
    delete static_cast<EditorAppImpl*>(pImpl_);
}

auto EditorApp::run() -> int {
#ifdef OKN_HAS_QT
    auto* impl = static_cast<EditorAppImpl*>(pImpl_);
    impl->qt_handle = create_qt_app(impl->argc, impl->argv);
    if (!impl->qt_handle) return 1;
    return run_qt_app(impl->qt_handle);
#else
    return 0;
#endif
}

} // namespace okn::editor
