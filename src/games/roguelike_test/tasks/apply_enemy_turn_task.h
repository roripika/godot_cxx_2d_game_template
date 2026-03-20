#pragma once
#include "core/tasks/task_base.h"

namespace karakuri::games::roguelike_test {

class ApplyEnemyTurnTask : public karakuri::TaskBase {
    GDCLASS(ApplyEnemyTurnTask, karakuri::TaskBase)

protected:
    static void _bind_methods();

public:
    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}

private:
    // no payload required
};

} // namespace karakuri::games::roguelike_test
