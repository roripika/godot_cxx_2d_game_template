#include "evaluate_rhythm_round_task.h"

#include "core/scenario/scenario_runner.h"
#include "core/world_state.h"

namespace karakuri::games::rhythm_test {

namespace {

constexpr int kScopeSession = karakuri::WorldState::SCOPE_SESSION;

} // namespace

void EvaluateRhythmRoundTask::_bind_methods() {}

godot::Error EvaluateRhythmRoundTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("if_clear") || !spec.payload.has("if_fail") || !spec.payload.has("if_continue")) {
        godot::UtilityFunctions::push_error(
            "[EvaluateRhythmRoundTask] 'if_clear', 'if_fail', and 'if_continue' are all required.");
        return godot::ERR_INVALID_DATA;
    }

    if_clear_ = spec.payload["if_clear"];
    if_fail_ = spec.payload["if_fail"];
    if_continue_ = spec.payload["if_continue"];
    return godot::OK;
}

TaskResult EvaluateRhythmRoundTask::execute() {
    auto *ws = karakuri::WorldState::get_singleton();
    if (!ws || !runner_) {
        godot::UtilityFunctions::push_error(
            "[EvaluateRhythmRoundTask] WorldState or runner is null.");
        return TaskResult::Failed;
    }

    const int chart_index = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "chart:index", 0));
    const int note_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "chart:note_count", 0));
    const int perfect_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "judge:perfect_count", 0));
    const int good_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "judge:good_count", 0));
    const int miss_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "judge:miss_count", 0));
    const int clear_hit_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "config:clear_hit_count", note_count));
    const int max_miss_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "config:max_miss_count", 1));

    godot::String round_result = "";
    godot::String round_status = "playing";
    if (miss_count > max_miss_count) {
        round_result = "fail";
        round_status = "failed";
    } else if (chart_index >= note_count) {
        if (perfect_count + good_count >= clear_hit_count) {
            round_result = "clear";
            round_status = "cleared";
        } else {
            round_result = "fail";
            round_status = "failed";
        }
    }

    ws->set_state("rhythm_test", kScopeSession, "round:result", round_result);
    ws->set_state("rhythm_test", kScopeSession, "round:status", round_status);

    godot::String target = if_continue_;
    if (round_result == "clear") {
        target = if_clear_;
    } else if (round_result == "fail") {
        target = if_fail_;
    }

    runner_->load_scene_by_id(target);
    return TaskResult::Success;
}

} // namespace karakuri::games::rhythm_test