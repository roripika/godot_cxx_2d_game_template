#ifndef KARAKURI_GAMES_BILLIARDS_TEST_RECORD_BILLIARDS_EVENT_TASK_H
#define KARAKURI_GAMES_BILLIARDS_TEST_RECORD_BILLIARDS_EVENT_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::billiards_test {

// Records a single billiards event into WorldState SCOPE_SESSION.
//
// Event effects (all writes to ns="billiards_test", SCOPE_SESSION):
//   shot_committed     → round:shots_fired += 1
//                        round:last_event  = "shot_committed"
//   ball_pocketed      → round:targets_pocketed += 1
//                        round:last_event  = "ball_pocketed"
//   cue_ball_pocketed  → round:foul        = true
//                        round:last_event  = "cue_ball_pocketed"
//   balls_stopped      → round:last_event  = "balls_stopped"
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
