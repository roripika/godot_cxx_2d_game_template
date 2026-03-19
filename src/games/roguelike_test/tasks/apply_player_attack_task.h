#ifndef KARAKURI_GAMES_ROGUELIKE_TEST_APPLY_PLAYER_ATTACK_TASK_H
#define KARAKURI_GAMES_ROGUELIKE_TEST_APPLY_PLAYER_ATTACK_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::roguelike_test {

// Applies a player attack against the nearest (only) enemy.
// The player can attack if enemy_1 is adjacent (Manhattan distance == 1).
//
// Required payload:
//   target  — "enemy_1"  (only valid target in initial version)
//
// Attack reduces enemy_1:hp by 1 (fixed damage = 1 in initial version).
// If enemy_1 is not adjacent, returns Failed.
// If enemy_1:hp reaches 0, writes round:result = "clear".
//
// Writes (SCOPE_SESSION, ns="roguelike_test"):
//   enemy_1:hp          — decremented by 1
//   round:result        — "clear" if enemy_1:hp == 0
//   last_action:type    = "attack"
//   last_action:actor   = "player"
class ApplyPlayerAttackTask : public TaskBase {
    GDCLASS(ApplyPlayerAttackTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String target_;
    ScenarioRunner *runner_ = nullptr;

public:
    ApplyPlayerAttackTask() = default;
    ~ApplyPlayerAttackTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::roguelike_test

#endif // KARAKURI_GAMES_ROGUELIKE_TEST_APPLY_PLAYER_ATTACK_TASK_H
