#ifndef KARAKURI_GAMES_BILLIARDS_TEST_WAIT_FOR_BILLIARDS_EVENT_TASK_H
#define KARAKURI_GAMES_BILLIARDS_TEST_WAIT_FOR_BILLIARDS_EVENT_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::billiards_test {

// Waits for a named billiards event or until the timeout elapses.
//
// Fake-event mode (current):
//   - Returns Waiting each frame until timeout_ seconds have elapsed.
//   - On timeout, writes round:last_event = "balls_stopped" to WorldState
//     and returns Success. This simulates a completed shot in offline testing.
//
// Future real-event mode (NOT implemented here):
//   - A physics callback layer would write the event to WorldState;
//     this task would detect it via WorldState::get_state and return Success.
//
// Validated event names: shot_committed, ball_pocketed,
//                         cue_ball_pocketed, balls_stopped.
class WaitForBilliardsEventTask : public TaskBase {
    GDCLASS(WaitForBilliardsEventTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::Array  expected_events_;  // valid event name strings to listen for
    double        timeout_    = 10.0;
    double        start_time_ = 0.0;
    ScenarioRunner *runner_ = nullptr;

public:
    WaitForBilliardsEventTask() = default;
    ~WaitForBilliardsEventTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override;
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::billiards_test

#endif // KARAKURI_GAMES_BILLIARDS_TEST_WAIT_FOR_BILLIARDS_EVENT_TASK_H
