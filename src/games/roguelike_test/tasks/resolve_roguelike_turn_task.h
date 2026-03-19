#pragma once
#include "games/task_base.h"

namespace karakuri::games::roguelike_test {

class ResolveRoguelikeTurnTask : public karakuri::games::TaskBase {
    GDCLASS(ResolveRoguelikeTurnTask, karakuri::games::TaskBase)

protected:
    static void _bind_methods();

public:
    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
};

} // namespace karakuri::games::roguelike_test
