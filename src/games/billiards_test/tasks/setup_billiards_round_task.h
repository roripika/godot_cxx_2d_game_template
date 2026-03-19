#ifndef KARAKURI_GAMES_BILLIARDS_TEST_SETUP_BILLIARDS_ROUND_TASK_H
#define KARAKURI_GAMES_BILLIARDS_TEST_SETUP_BILLIARDS_ROUND_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::billiards_test {

// Initialises a billiards round by writing logical round state to
// WorldState SCOPE_SESSION under the namespace "billiards_test".
//
// Keys written:
//   billiards_test  SCOPE_SESSION  round:shots_taken        = 0
//   billiards_test  SCOPE_SESSION  round:shot_limit         = shot_limit  (config)
//   billiards_test  SCOPE_SESSION  round:target_count       = target_count (config)
//   billiards_test  SCOPE_SESSION  round:target_1_pocketed  = false
//   billiards_test  SCOPE_SESSION  round:target_2_pocketed  = false
//   billiards_test  SCOPE_SESSION  round:cue_ball_pocketed  = false
//   billiards_test  SCOPE_SESSION  round:result             = ""
//   billiards_test  SCOPE_SESSION  event:last_name          = ""
//
// Does NOT store physical coordinates or velocities.
class SetupBilliardsRoundTask : public TaskBase {
    GDCLASS(SetupBilliardsRoundTask, TaskBase)

protected:
    static void _bind_methods();

private:
    int shot_limit_    = 5;
    int target_count_  = 2;
    ScenarioRunner *runner_ = nullptr;

public:
    SetupBilliardsRoundTask() = default;
    ~SetupBilliardsRoundTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::billiards_test

#endif // KARAKURI_GAMES_BILLIARDS_TEST_SETUP_BILLIARDS_ROUND_TASK_H
