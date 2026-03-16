#ifndef KARAKURI_MYSTERY_TEST_SHOW_DIALOGUE_TASK_H
#define KARAKURI_MYSTERY_TEST_SHOW_DIALOGUE_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri {

class ShowDialogueTask : public TaskBase {
    GDCLASS(ShowDialogueTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String speaker_;
    godot::String text_;
    ScenarioRunner *runner_ = nullptr;

public:
    ShowDialogueTask() = default;
    ~ShowDialogueTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri

#endif // KARAKURI_MYSTERY_TEST_SHOW_DIALOGUE_TASK_H
