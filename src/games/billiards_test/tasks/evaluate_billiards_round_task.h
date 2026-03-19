#ifndef KARAKURI_GAMES_BILLIARDS_TEST_EVALUATE_BILLIARDS_ROUND_TASK_H
#define KARAKURI_GAMES_BILLIARDS_TEST_EVALUATE_BILLIARDS_ROUND_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::billiards_test {

// Reads round state from WorldState SCOPE_SESSION and jumps to the
// appropriate scene via ScenarioRunner::load_scene_by_id().
//
// Evaluation priority (checked in order):
//   1. round:cue_ball_pocketed == true               → if_fail
//   2. (target_1_pocketed + target_2_pocketed) >= target_count → if_clear
//   3. round:shots_taken >= round:shot_limit          → if_fail
//   4. otherwise                                     → if_continue
//
// Writes round:result = "clear" / "fail" / "active" accordingly.
// Returns Failed if WorldState or runner is null.
class EvaluateBilliardsRoundTask : public TaskBase {
    GDCLASS(EvaluateBilliardsRoundTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String if_clear_;
    godot::String if_fail_;
    godot::String if_continue_;
    ScenarioRunner *runner_ = nullptr;

public:
    EvaluateBilliardsRoundTask() = default;
    ~EvaluateBilliardsRoundTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::billiards_test

#endif // KARAKURI_GAMES_BILLIARDS_TEST_EVALUATE_BILLIARDS_ROUND_TASK_H
