#pragma once

#include <string>
#include <vector>
#include <memory>

namespace okn::editor {

class ICommand;

class CommandManager {
public:
    CommandManager();
    ~CommandManager();

    auto execute(std::unique_ptr<ICommand> cmd) -> void;
    auto undo() -> void;
    auto redo() -> void;
    [[nodiscard]] auto can_undo() const -> bool;
    [[nodiscard]] auto can_redo() const -> bool;
    [[nodiscard]] auto undo_description() const -> std::string;
    [[nodiscard]] auto redo_description() const -> std::string;
    auto clear() -> void;

private:
    void* pImpl_ = nullptr;
};

} // namespace okn::editor
