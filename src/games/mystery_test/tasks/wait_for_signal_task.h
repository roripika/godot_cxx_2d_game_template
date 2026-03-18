#ifndef KARAKURI_GAMES_MYSTERY_TEST_WAIT_FOR_SIGNAL_TASK_H
#define KARAKURI_GAMES_MYSTERY_TEST_WAIT_FOR_SIGNAL_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::mystery_test {

class WaitForSignalTask : public TaskBase {
    GDCLASS(WaitForSignalTask, TaskBase)

protected:
    static void _bind_methods();

private:
    double timeout_ = 5.0;
    double start_time_ = 0.0;
    godot::String signal_name_;
    ScenarioRunner *runner_ = nullptr;

public:
    WaitForSignalTask() = default;
    ~WaitForSignalTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner* runner) override { runner_ = runner; }
};

} // namespace karakuri::games::mystery_test

#endif // KARAKURI_GAMES_MYSTERY_TEST_WAIT_FOR_SIGNAL_TASK_H
