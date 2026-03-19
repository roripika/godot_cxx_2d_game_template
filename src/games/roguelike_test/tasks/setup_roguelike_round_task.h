#ifndef KARAKURI_GAMES_ROGUELIKE_TEST_SETUP_ROGUELIKE_ROUND_TASK_H
#define KARAKURI_GAMES_ROGUELIKE_TEST_SETUP_ROGUELIKE_ROUND_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::roguelike_test {

// Initialises a roguelike round by writing logical state to
// WorldState SCOPE_SESSION under the namespace "roguelike_test".
//
// Fixed 5x5 map. Wall cells are implicit from the 5x5 boundary.
// No procedural generation. No mutable map cells in WorldState.
//
// Keys written (all SCOPE_SESSION, ns="roguelike_test"):
//   turn:index         = 0
//   turn:phase         = "player"
//   player:x           = player_start_x  (default 1)
//   player:y           = player_start_y  (default 1)
//   player:hp          = player_hp       (default 3)
//   enemy_1:x          = enemy_start_x   (default 3)
//   enemy_1:y          = enemy_start_y   (default 3)
//   enemy_1:hp         = enemy_hp        (default 2)
//   goal:x             = goal_x          (default 4)
//   goal:y             = goal_y          (default 4)
//   round:result       = ""
//   last_action:actor  = ""
//   last_action:type   = ""
//
// Optional payload keys:
//   player_hp    (default 3)   enemy_hp   (default 2)
//   player_x     (default 1)   player_y   (default 1)
//   enemy_1_x    (default 3)   enemy_1_y  (default 3)
class SetupRoguelikeRoundTask : public TaskBase {
    GDCLASS(SetupRoguelikeRoundTask, TaskBase)

protected:
    static void _bind_methods();

private:
    int player_hp_  = 3;
    int enemy_hp_   = 2;
    int player_x_   = 1;
    int player_y_   = 1;
    int enemy_1_x_  = 3;
    int enemy_1_y_  = 3;
    ScenarioRunner *runner_ = nullptr;

public:
    SetupRoguelikeRoundTask() = default;
    ~SetupRoguelikeRoundTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::roguelike_test

#endif // KARAKURI_GAMES_ROGUELIKE_TEST_SETUP_ROGUELIKE_ROUND_TASK_H
