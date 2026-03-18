#ifndef KARAKURI_GAMES_MYSTERY_TEST_PARALLEL_TASK_GROUP_H
#define KARAKURI_GAMES_MYSTERY_TEST_PARALLEL_TASK_GROUP_H

#include "core/tasks/task_base.h"
#include <godot_cpp/classes/ref.hpp>
#include <vector>

namespace karakuri::games::mystery_test {

class ParallelTaskGroup : public TaskBase {
    GDCLASS(ParallelTaskGroup, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::TypedArray<TaskBase> sub_tasks_;
    std::vector<bool> finished_;
    ScenarioRunner *runner_ = nullptr;

public:
    ParallelTaskGroup() = default;
    ~ParallelTaskGroup() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override;
    void set_runner(ScenarioRunner* runner) override;
};

} // namespace karakuri::games::mystery_test

#endif // KARAKURI_GAMES_MYSTERY_TEST_PARALLEL_TASK_GROUP_H
