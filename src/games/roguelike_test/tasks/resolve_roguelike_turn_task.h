#pragma once
#include "core/tasks/task_base.h"

namespace karakuri::games::roguelike_test {

class ResolveRoguelikeTurnTask : public karakuri::TaskBase {
    GDCLASS(ResolveRoguelikeTurnTask, karakuri::TaskBase)

protected:
    static void _bind_methods();

public:
    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
};

} // namespace karakuri::games::roguelike_test
