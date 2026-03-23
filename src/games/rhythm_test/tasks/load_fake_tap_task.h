#pragma once

#include "core/tasks/task_base.h"

namespace karakuri::games::rhythm_test {

class LoadFakeTapTask : public TaskBase {
    GDCLASS(LoadFakeTapTask, TaskBase)

protected:
    static void _bind_methods();

public:
    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
};

} // namespace karakuri::games::rhythm_test