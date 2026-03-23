#pragma once

#include "core/tasks/task_base.h"

namespace karakuri::games::rhythm_test {

class EvaluateRhythmRoundTask : public TaskBase {
    GDCLASS(EvaluateRhythmRoundTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String if_clear_;
    godot::String if_fail_;
    godot::String if_continue_;
    ScenarioRunner *runner_ = nullptr;

public:
    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::rhythm_test