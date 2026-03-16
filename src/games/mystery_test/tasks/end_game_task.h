#ifndef KARAKURI_MYSTERY_TEST_END_GAME_TASK_H
#define KARAKURI_MYSTERY_TEST_END_GAME_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri {

class EndGameTask : public TaskBase {
    GDCLASS(EndGameTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String result_;
    ScenarioRunner *runner_ = nullptr;

public:
    EndGameTask() = default;
    ~EndGameTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri

#endif // KARAKURI_MYSTERY_TEST_END_GAME_TASK_H
