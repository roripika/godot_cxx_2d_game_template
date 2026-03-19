#ifndef KARAKURI_GAMES_BILLIARDS_TEST_RECORD_BILLIARDS_EVENT_TASK_H
#define KARAKURI_GAMES_BILLIARDS_TEST_RECORD_BILLIARDS_EVENT_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::billiards_test {

// Records a single billiards event into WorldState SCOPE_SESSION.
//
// Event effects (all writes to ns="billiards_test", SCOPE_SESSION):
//   shot_committed     → round:shots_taken += 1
//                        event:last_name  = "shot_committed"
//   ball_pocketed      → round:target_1_pocketed = true (first call)
//                        round:target_2_pocketed = true (second call)
//                        event:last_name  = "ball_pocketed"
//   cue_ball_pocketed  → round:cue_ball_pocketed = true
//                        event:last_name  = "cue_ball_pocketed"
//   balls_stopped      → event:last_name  = "balls_stopped"
//
// Returns Failed if WorldState is null or the event is unknown.
class RecordBilliardsEventTask : public TaskBase {
    GDCLASS(RecordBilliardsEventTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String event_;
    ScenarioRunner *runner_ = nullptr;

public:
    RecordBilliardsEventTask() = default;
    ~RecordBilliardsEventTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::billiards_test

#endif // KARAKURI_GAMES_BILLIARDS_TEST_RECORD_BILLIARDS_EVENT_TASK_H
