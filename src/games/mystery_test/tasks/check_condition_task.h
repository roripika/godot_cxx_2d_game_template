#ifndef KARAKURI_GAMES_MYSTERY_TEST_CHECK_CONDITION_TASK_H
#define KARAKURI_GAMES_MYSTERY_TEST_CHECK_CONDITION_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::mystery_test {

// Evaluates an all_of or any_of condition against WorldState SESSION scope
// and jumps to if_true / if_false via ScenarioRunner::load_scene_by_id().
class CheckConditionTask : public TaskBase {
    GDCLASS(CheckConditionTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::Array  conditions_;   // validated list of {evidence: <id>} dicts
    bool          use_all_of_ = true;
    godot::String if_true_;
    godot::String if_false_;
    ScenarioRunner *runner_ = nullptr;

public:
    CheckConditionTask() = default;
    ~CheckConditionTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::mystery_test

#endif // KARAKURI_GAMES_MYSTERY_TEST_CHECK_CONDITION_TASK_H
