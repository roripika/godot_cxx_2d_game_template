#pragma once
#include "games/task_base.h"

namespace karakuri::games::roguelike_test {

// Reads roguelike_test:round:result from WorldState and branches to
// the appropriate scene via ScenarioRunner::load_scene_by_id().
//
// Branches (payload keys required):
//   round:result == "clear"  → if_clear
//   round:result == "fail"   → if_fail
//   otherwise                → if_continue
class EvaluateRoguelikeRoundTask : public karakuri::games::TaskBase {
    GDCLASS(EvaluateRoguelikeRoundTask, karakuri::games::TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String if_clear_;
    godot::String if_fail_;
    godot::String if_continue_;
    ScenarioRunner *runner_ = nullptr;

public:
    EvaluateRoguelikeRoundTask() = default;
    ~EvaluateRoguelikeRoundTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::roguelike_test
