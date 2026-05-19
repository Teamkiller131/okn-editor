#include <okn/editor/commands/history.hpp>
#include <okn/editor/config.hpp>

namespace okn::editor {

auto CommandHistory::push(std::unique_ptr<ICommand> cmd) -> void {
    cmd->execute();
    done_.push_back(std::move(cmd));
    undone_.clear();
    if (done_.size() > kMaxUndoSteps) {
        done_.erase(done_.begin());
    }
}

auto CommandHistory::undo() -> void {
    if (done_.empty()) return;
    auto cmd = std::move(done_.back());
    done_.pop_back();
    cmd->undo();
    undone_.push_back(std::move(cmd));
}

auto CommandHistory::redo() -> void {
    if (undone_.empty()) return;
    auto cmd = std::move(undone_.back());
    undone_.pop_back();
    cmd->execute();
    done_.push_back(std::move(cmd));
}

auto CommandHistory::can_undo() const -> bool { return !done_.empty(); }

auto CommandHistory::can_redo() const -> bool { return !undone_.empty(); }

auto CommandHistory::clear() -> void {
    done_.clear();
    undone_.clear();
}

auto CommandHistory::size() const -> size_t { return done_.size(); }

} // namespace okn::editor
