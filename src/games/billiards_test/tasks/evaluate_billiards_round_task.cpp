#include "evaluate_billiards_round_task.h"

#include "core/world_state.h"
#include "core/scenario/scenario_runner.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::billiards_test {

void EvaluateBilliardsRoundTask::_bind_methods() {}

godot::Error EvaluateBilliardsRoundTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("if_clear") || !spec.payload.has("if_fail") || !spec.payload.has("if_continue")) {
        godot::UtilityFunctions::push_error(
            "[EvaluateBilliardsRoundTask] 'if_clear', 'if_fail', and 'if_continue' are all required.");
        return godot::ERR_INVALID_DATA;
    }
    if_clear_    = spec.payload["if_clear"];
    if_fail_     = spec.payload["if_fail"];
    if_continue_ = spec.payload["if_continue"];
    return godot::OK;
}

TaskResult EvaluateBilliardsRoundTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws || !runner_) {
        godot::UtilityFunctions::push_error("[EvaluateBilliardsRoundTask] WorldState or runner is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;

    bool cue_foul    = static_cast<bool>(ws->get_state("billiards_test", S, "round:cue_ball_pocketed", false));
    bool t1          = static_cast<bool>(ws->get_state("billiards_test", S, "round:target_1_pocketed", false));
    bool t2          = static_cast<bool>(ws->get_state("billiards_test", S, "round:target_2_pocketed", false));
    int  pocketed    = (t1 ? 1 : 0) + (t2 ? 1 : 0);
    int  target_count = static_cast<int>(ws->get_state("billiards_test", S, "round:target_count",      1));
    int  shots_taken  = static_cast<int>(ws->get_state("billiards_test", S, "round:shots_taken",       0));
    int  shot_limit   = static_cast<int>(ws->get_state("billiards_test", S, "round:shot_limit",        5));

    godot::String target;
    if (cue_foul) {
        target = if_fail_;
        godot::UtilityFunctions::print("[EvaluateBilliardsRoundTask] Cue-ball foul → ", target);
    } else if (pocketed >= target_count) {
        target = if_clear_;
        godot::UtilityFunctions::print("[EvaluateBilliardsRoundTask] Clear → ", target);
    } else if (shots_taken >= shot_limit) {
        target = if_fail_;
        godot::UtilityFunctions::print("[EvaluateBilliardsRoundTask] Shot limit reached → ", target);
    } else {
        target = if_continue_;
        godot::UtilityFunctions::print("[EvaluateBilliardsRoundTask] Continue → ", target);
    }

    ws->set_state("billiards_test", S, "round:result",
        (target == if_clear_) ? godot::String("clear")
        : (target == if_fail_) ? godot::String("fail")
        : godot::String("active"));

    runner_->load_scene_by_id(target);
    return TaskResult::Success;
}

} // namespace karakuri::games::billiards_test
