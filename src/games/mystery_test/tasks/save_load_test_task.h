#ifndef KARAKURI_GAMES_MYSTERY_TEST_SAVE_LOAD_TEST_TASK_H
#define KARAKURI_GAMES_MYSTERY_TEST_SAVE_LOAD_TEST_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::mystery_test {

class SaveLoadTestTask : public TaskBase {
    GDCLASS(SaveLoadTestTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String test_key_ = "savetest_flag";
    ScenarioRunner *runner_ = nullptr;

public:
    SaveLoadTestTask() = default;
    ~SaveLoadTestTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner* runner) override { runner_ = runner; }
};

} // namespace karakuri::games::mystery_test

#endif // KARAKURI_GAMES_MYSTERY_TEST_SAVE_LOAD_TEST_TASK_H
