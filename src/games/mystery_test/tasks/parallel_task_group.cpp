#include "parallel_task_group.h"
#include "core/action_registry.h"
#include "core/scenario/scenario_runner.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::mystery_test {

void ParallelTaskGroup::_bind_methods() {}

godot::Error ParallelTaskGroup::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("tasks")) {
        godot::UtilityFunctions::push_error("[ParallelTaskGroup] 'tasks' array is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }

    godot::Array task_list = spec.payload["tasks"];
    auto registry = ActionRegistry::get_singleton();
    
    for (int i = 0; i < task_list.size(); ++i) {
        godot::Dictionary task_data = task_list[i];
        TaskSpec sub_spec(
            task_data.get("action", ""),
            task_data.get("payload", godot::Dictionary())
        );
        
        godot::Ref<TaskBase> sub_task = registry->compile_task(sub_spec);
        if (sub_task.is_valid()) {
            sub_tasks_.append(sub_task);
            finished_.push_back(false);
        } else {
            godot::UtilityFunctions::push_error("[ParallelTaskGroup] Failed to compile sub-task: ", sub_spec.action);
            return godot::ERR_INVALID_DATA;
        }
    }

    return godot::OK;
}

TaskResult ParallelTaskGroup::execute() {
    bool all_finished = true;
    bool any_failed = false;

    for (int i = 0; i < sub_tasks_.size(); ++i) {
        if (finished_[i]) continue;

        godot::Ref<TaskBase> task = sub_tasks_[i];
        TaskResult res = task->execute();

        if (res == TaskResult::Success) {
            finished_[i] = true;
        } else if (res == TaskResult::Failed) {
            finished_[i] = true;
            any_failed = true;
        } else {
            all_finished = false;
        }
    }

    if (any_failed) return TaskResult::Failed;
    if (all_finished) return TaskResult::Success;
    
    return TaskResult::Waiting;
}

void ParallelTaskGroup::complete_instantly() {
    for (int i = 0; i < sub_tasks_.size(); ++i) {
        godot::Ref<TaskBase> task = sub_tasks_[i];
        task->complete_instantly();
    }
}

void ParallelTaskGroup::set_runner(ScenarioRunner* runner) {
    runner_ = runner;
    for (int i = 0; i < sub_tasks_.size(); ++i) {
        godot::Ref<TaskBase> task = sub_tasks_[i];
        task->set_runner(runner);
    }
}

} // namespace karakuri::games::mystery_test
