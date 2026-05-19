#pragma once

#include <string>
#include <vector>

namespace okn::editor {

struct FileEntry {
    std::string name;
    std::string path;
    bool is_directory = false;
};

class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager();

    auto new_project(const std::string& path, const std::string& name) -> bool;
    auto open_project(const std::string& path) -> bool;
    auto save_project() -> bool;
    auto close_project() -> void;
    [[nodiscard]] auto is_open() const -> bool;

    [[nodiscard]] auto file_tree(const std::string& sub_path = "") const -> std::vector<FileEntry>;
    [[nodiscard]] auto project_path() const -> std::string;
    [[nodiscard]] auto project_name() const -> std::string;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
