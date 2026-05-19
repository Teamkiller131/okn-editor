#include <okn/editor/command_manager.hpp>
#include <okn/editor/commands/history.hpp>
#include <okn/editor/commands/command.hpp>

namespace okn::editor {

struct CommandManagerImpl {
    CommandHistory history;
};

CommandManager::CommandManager() : pImpl_(new CommandManagerImpl()) {}
CommandManager::~CommandManager() { delete static_cast<CommandManagerImpl*>(pImpl_); }

auto CommandManager::execute(std::unique_ptr<ICommand> cmd) -> void {
    static_cast<CommandManagerImpl*>(pImpl_)->history.push(std::move(cmd));
}

auto CommandManager::undo() -> void {
    static_cast<CommandManagerImpl*>(pImpl_)->history.undo();
}

auto CommandManager::redo() -> void {
    static_cast<CommandManagerImpl*>(pImpl_)->history.redo();
}

auto CommandManager::can_undo() const -> bool {
    return static_cast<const CommandManagerImpl*>(pImpl_)->history.can_undo();
}

auto CommandManager::can_redo() const -> bool {
    return static_cast<const CommandManagerImpl*>(pImpl_)->history.can_redo();
}

auto CommandManager::undo_description() const -> std::string {
    return can_undo() ? "Undo" : "";
}

auto CommandManager::redo_description() const -> std::string {
    return can_redo() ? "Redo" : "";
}

auto CommandManager::clear() -> void {
    static_cast<CommandManagerImpl*>(pImpl_)->history.clear();
}

} // namespace okn::editor
