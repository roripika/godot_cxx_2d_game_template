#pragma once
#include "games/task_base.h"

namespace karakuri::games::roguelike_test {

class ApplyEnemyTurnTask : public karakuri::games::TaskBase {
    GDCLASS(ApplyEnemyTurnTask, karakuri::games::TaskBase)

protected:
    static void _bind_methods();

public:
    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;

private:
    // no payload required
};

} // namespace karakuri::games::roguelike_test
