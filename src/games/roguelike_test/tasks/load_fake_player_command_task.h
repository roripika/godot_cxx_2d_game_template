#ifndef KARAKURI_GAMES_ROGUELIKE_TEST_LOAD_FAKE_PLAYER_COMMAND_TASK_H
#define KARAKURI_GAMES_ROGUELIKE_TEST_LOAD_FAKE_PLAYER_COMMAND_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::roguelike_test {

// Loads a scripted (fake) player command into WorldState so that
// apply_player_move / apply_player_attack can read and execute it.
//
// In the initial version there is no real input bridge; all player
// actions are scripted from the YAML payload. This keeps the module
// deterministic and physics-free.
//
// Required payload:
//   command  — one of: "move_up" | "move_down" | "move_left" | "move_right"
//                       | "attack"
//
// Keys written (SCOPE_SESSION, ns="roguelike_test"):
//   last_action:type  = command
//   last_action:actor = "player"
//   turn:phase        = "player_act"
class LoadFakePlayerCommandTask : public TaskBase {
    GDCLASS(LoadFakePlayerCommandTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String command_;
    ScenarioRunner *runner_ = nullptr;

public:
    LoadFakePlayerCommandTask() = default;
    ~LoadFakePlayerCommandTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::roguelike_test

#endif // KARAKURI_GAMES_ROGUELIKE_TEST_LOAD_FAKE_PLAYER_COMMAND_TASK_H
