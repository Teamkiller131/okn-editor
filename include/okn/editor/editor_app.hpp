#pragma once

namespace okn::editor {

class EditorApp {
public:
    EditorApp(int argc, char* argv[]);
    ~EditorApp();

    auto run() -> int;

    EditorApp(const EditorApp&) = delete;
    auto operator=(const EditorApp&) -> EditorApp& = delete;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
