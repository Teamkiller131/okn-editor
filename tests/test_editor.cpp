#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <okn/editor/editor_engine.hpp>
#include <okn/editor/command_manager.hpp>
#include <okn/editor/project/project_manager.hpp>
#include <okn/editor/commands/command.hpp>
#include <okn/editor/commands/history.hpp>
#include <memory>
#include <string>

using namespace okn::editor;

// ── Test command for undo/redo ──

struct TestCommand : ICommand {
    int* value = nullptr;
    int old_val = 0;
    int new_val = 0;

    TestCommand(int* v, int nv) : value(v), old_val(*v), new_val(nv) {}

    auto execute() -> void override { *value = new_val; }
    auto undo() -> void override { *value = old_val; }
    auto description() const -> const std::string& override { static std::string d = "test command"; return d; }
};

// ── Editor Engine ──

TEST_CASE("okn::editor::EditorEngine - singleton") {
    auto& e1 = EditorEngine::instance();
    auto& e2 = EditorEngine::instance();
    CHECK(&e1 == &e2);
}

TEST_CASE("okn::editor::EditorEngine - init lifecycle") {
    auto& engine = EditorEngine::instance();
    if (!engine.is_initialized()) {
        engine.initialize();
    }
    CHECK(engine.is_initialized());
}

TEST_CASE("okn::editor::EditorEngine - project") {
    auto& engine = EditorEngine::instance();
    [[maybe_unused]] auto& proj = engine.project_manager();
    CHECK(true); // ProjectManager reference is valid
}

// ── Command Manager ──

TEST_CASE("okn::editor::CommandManager - execute undo redo") {
    CommandManager cm;
    int value = 10;

    auto cmd = std::make_unique<TestCommand>(&value, 42);
    cm.execute(std::move(cmd));
    CHECK(value == 42);
    CHECK(cm.can_undo());
    CHECK(!cm.can_redo());

    cm.undo();
    CHECK(value == 10);
    CHECK(!cm.can_undo());
    CHECK(cm.can_redo());

    cm.redo();
    CHECK(value == 42);
}

TEST_CASE("okn::editor::CommandManager - multiple commands") {
    CommandManager cm;
    int value = 0;

    cm.execute(std::make_unique<TestCommand>(&value, 1));
    cm.execute(std::make_unique<TestCommand>(&value, 2));
    cm.execute(std::make_unique<TestCommand>(&value, 3));
    CHECK(value == 3);

    cm.undo();
    CHECK(value == 2);
    cm.undo();
    CHECK(value == 1);
    cm.undo();
    CHECK(value == 0);
    CHECK(!cm.can_undo());
}

// ── Command History ──

TEST_CASE("okn::editor::CommandHistory - push pop") {
    CommandHistory history;
    CHECK(history.size() == 0);
    CHECK(!history.can_undo());

    int value = 0;
    history.push(std::make_unique<TestCommand>(&value, 100));
    CHECK(value == 100);
    CHECK(history.can_undo());

    history.undo();
    CHECK(value == 0);
}

TEST_CASE("okn::editor::CommandHistory - redo") {
    CommandHistory history;
    int value = 5;
    history.push(std::make_unique<TestCommand>(&value, 15));
    history.undo();
    history.redo();
    CHECK(value == 15);
}
