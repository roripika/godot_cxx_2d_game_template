#ifndef KARAKURI_GAMES_ROGUELIKE_TEST_APPLY_PLAYER_MOVE_TASK_H
#define KARAKURI_GAMES_ROGUELIKE_TEST_APPLY_PLAYER_MOVE_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::roguelike_test {

// Reads last_action:type from WorldState and moves the player one cell
// in the indicated direction, subject to:
//   - Grid boundary check (map is 5x5, cells 0-4 on each axis)
//   - Occupancy check: cannot move into enemy_1's cell
//
// If the destination is out of bounds or occupied, returns Failed.
// Leaves player position unchanged on failure.
//
// No payload required. Reads from:
//   last_action:type  — direction command set by LoadFakePlayerCommandTask
//
// Writes (SCOPE_SESSION, ns="roguelike_test"):
//   player:x, player:y   — updated position
//   round:result         — set to "clear" if player reaches goal cell
class ApplyPlayerMoveTask : public TaskBase {
    GDCLASS(ApplyPlayerMoveTask, TaskBase)

protected:
    static void _bind_methods();

private:
    ScenarioRunner *runner_ = nullptr;

public:
    ApplyPlayerMoveTask() = default;
    ~ApplyPlayerMoveTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::roguelike_test

#endif // KARAKURI_GAMES_ROGUELIKE_TEST_APPLY_PLAYER_MOVE_TASK_H
