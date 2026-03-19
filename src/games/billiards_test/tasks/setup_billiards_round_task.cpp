#include "setup_billiards_round_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::billiards_test {

void SetupBilliardsRoundTask::_bind_methods() {}

godot::Error SetupBilliardsRoundTask::validate_and_setup(const TaskSpec &spec) {
    if (spec.payload.has("shot_limit")) {
        shot_limit_ = static_cast<int>(spec.payload["shot_limit"]);
        if (shot_limit_ <= 0) {
            godot::UtilityFunctions::push_error("[SetupBilliardsRoundTask] 'shot_limit' must be > 0.");
            return godot::ERR_INVALID_DATA;
        }
    }
    if (spec.payload.has("target_count")) {
        target_count_ = static_cast<int>(spec.payload["target_count"]);
        if (target_count_ <= 0) {
            godot::UtilityFunctions::push_error("[SetupBilliardsRoundTask] 'target_count' must be > 0.");
            return godot::ERR_INVALID_DATA;
        }
    }
    return godot::OK;
}

TaskResult SetupBilliardsRoundTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error("[SetupBilliardsRoundTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;
    ws->set_state("billiards_test", S, "round:shots_taken",       0);
    ws->set_state("billiards_test", S, "round:shot_limit",        shot_limit_);
    ws->set_state("billiards_test", S, "round:target_count",      target_count_);
    ws->set_state("billiards_test", S, "round:target_1_pocketed", false);
    ws->set_state("billiards_test", S, "round:target_2_pocketed", false);
    ws->set_state("billiards_test", S, "round:cue_ball_pocketed", false);
    ws->set_state("billiards_test", S, "round:result",            godot::String(""));
    ws->set_state("billiards_test", S, "event:last_name",         godot::String(""));

    godot::UtilityFunctions::print(
        "[SetupBilliardsRoundTask] Round initialised: shot_limit=", shot_limit_,
        " target_count=", target_count_);

    return TaskResult::Success;
}

} // namespace karakuri::games::billiards_test
