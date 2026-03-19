#include "evaluate_roguelike_round_task.h"

#include "core/world_state.h"
#include "core/scenario/scenario_runner.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::roguelike_test {

void EvaluateRoguelikeRoundTask::_bind_methods() {}

godot::Error EvaluateRoguelikeRoundTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("if_clear") || !spec.payload.has("if_fail") || !spec.payload.has("if_continue")) {
        godot::UtilityFunctions::push_error(
            "[EvaluateRoguelikeRoundTask] 'if_clear', 'if_fail', and 'if_continue' are all required.");
        return godot::ERR_INVALID_DATA;
    }
    if_clear_    = spec.payload["if_clear"];
    if_fail_     = spec.payload["if_fail"];
    if_continue_ = spec.payload["if_continue"];
    return godot::OK;
}

TaskResult EvaluateRoguelikeRoundTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws || !runner_) {
        godot::UtilityFunctions::push_error(
            "[EvaluateRoguelikeRoundTask] WorldState or runner is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;
    godot::String result = ws->get_state("roguelike_test", S, "round:result", godot::String(""));

    godot::String target;
    if (result == "clear") {
        target = if_clear_;
        godot::UtilityFunctions::print("[EvaluateRoguelikeRoundTask] Clear → ", target);
    } else if (result == "fail") {
        target = if_fail_;
        godot::UtilityFunctions::print("[EvaluateRoguelikeRoundTask] Fail → ", target);
    } else {
        target = if_continue_;
        godot::UtilityFunctions::print("[EvaluateRoguelikeRoundTask] Continue → ", target);
    }

    runner_->load_scene_by_id(target);
    return TaskResult::Success;
}

} // namespace karakuri::games::roguelike_test
