#include <okn/editor/project/project_manager.hpp>

namespace okn::editor {

struct ProjectManagerImpl {
    std::string project_path;
    std::string project_name;
    bool is_open = false;
};

ProjectManager::ProjectManager() : pImpl_(new ProjectManagerImpl()) {}
ProjectManager::~ProjectManager() { delete static_cast<ProjectManagerImpl*>(pImpl_); }

auto ProjectManager::new_project(const std::string& path, const std::string& name) -> bool {
    auto* impl = static_cast<ProjectManagerImpl*>(pImpl_);
    impl->project_path = path;
    impl->project_name = name;
    impl->is_open = true;
    return true;
}

auto ProjectManager::open_project(const std::string& path) -> bool {
    auto* impl = static_cast<ProjectManagerImpl*>(pImpl_);
    impl->project_path = path;
    auto slash = path.find_last_of("/\\");
    impl->project_name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    impl->is_open = true;
    return true;
}

auto ProjectManager::save_project() -> bool {
    return static_cast<ProjectManagerImpl*>(pImpl_)->is_open;
}

auto ProjectManager::close_project() -> void {
    auto* impl = static_cast<ProjectManagerImpl*>(pImpl_);
    impl->is_open = false;
    impl->project_path.clear();
    impl->project_name.clear();
}

auto ProjectManager::is_open() const -> bool {
    return static_cast<const ProjectManagerImpl*>(pImpl_)->is_open;
}

auto ProjectManager::file_tree(const std::string& sub_path) const -> std::vector<FileEntry> {
    return {};
}

auto ProjectManager::project_path() const -> std::string {
    return static_cast<const ProjectManagerImpl*>(pImpl_)->project_path;
}

auto ProjectManager::project_name() const -> std::string {
    return static_cast<const ProjectManagerImpl*>(pImpl_)->project_name;
}

} // namespace okn::editor
