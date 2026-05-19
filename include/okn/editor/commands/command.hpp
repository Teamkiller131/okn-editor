#pragma once

#include <string>
#include <memory>

namespace okn::editor {

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual auto execute() -> void = 0;
    virtual auto undo() -> void = 0;
    virtual auto description() const -> const std::string& = 0;
};

} // namespace okn::editor
