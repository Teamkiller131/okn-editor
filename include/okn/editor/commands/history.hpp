#pragma once

#include <vector>
#include <memory>
#include <okn/editor/commands/command.hpp>

namespace okn::editor {

class CommandHistory {
public:
    CommandHistory() = default;
    ~CommandHistory() = default;

    auto push(std::unique_ptr<ICommand> cmd) -> void;
    auto undo() -> void;
    auto redo() -> void;
    [[nodiscard]] auto can_undo() const -> bool;
    [[nodiscard]] auto can_redo() const -> bool;
    auto clear() -> void;
    [[nodiscard]] auto size() const -> size_t;

private:
    std::vector<std::unique_ptr<ICommand>> done_;
    std::vector<std::unique_ptr<ICommand>> undone_;
};

} // namespace okn::editor
