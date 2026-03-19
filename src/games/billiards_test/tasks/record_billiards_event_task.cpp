#include "record_billiards_event_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::billiards_test {

void RecordBilliardsEventTask::_bind_methods() {}

godot::Error RecordBilliardsEventTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("event")) {
        godot::UtilityFunctions::push_error("[RecordBilliardsEventTask] 'event' is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }
    const godot::String ev = spec.payload["event"];

    // Validate against the canonical list.
    static const char* VALID[] = {
        "shot_committed", "ball_pocketed", "cue_ball_pocketed", "balls_stopped"};
    bool found = false;
    for (const char* v : VALID) {
        if (ev == v) { found = true; break; }
    }
    if (!found) {
        godot::UtilityFunctions::push_error("[RecordBilliardsEventTask] Unknown event: ", ev);
        return godot::ERR_INVALID_DATA;
    }

    event_ = ev;
    return godot::OK;
}

TaskResult RecordBilliardsEventTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error("[RecordBilliardsEventTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;

    if (event_ == "shot_committed") {
        int taken = static_cast<int>(ws->get_state("billiards_test", S, "round:shots_taken", 0));
        ws->set_state("billiards_test", S, "round:shots_taken", taken + 1);
    } else if (event_ == "ball_pocketed") {
        // Fill target_1 first, then target_2 (max 2 object balls in this test)
        bool t1 = static_cast<bool>(ws->get_state("billiards_test", S, "round:target_1_pocketed", false));
        if (!t1) {
            ws->set_state("billiards_test", S, "round:target_1_pocketed", true);
        } else {
            ws->set_state("billiards_test", S, "round:target_2_pocketed", true);
        }
    } else if (event_ == "cue_ball_pocketed") {
        ws->set_state("billiards_test", S, "round:cue_ball_pocketed", true);
    }
    // balls_stopped: no extra state changes

    ws->set_state("billiards_test", S, "event:last_name", event_);
    godot::UtilityFunctions::print("[RecordBilliardsEventTask] Recorded event: ", event_);

    return TaskResult::Success;
}

} // namespace karakuri::games::billiards_test
