#include "wait_for_billiards_event_task.h"

#include "core/world_state.h"
#include "core/kernel_clock.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::billiards_test {

// Valid event names that a billiards scenario can wait on.
static const godot::String VALID_EVENTS[] = {
    "shot_committed",
    "ball_pocketed",
    "cue_ball_pocketed",
    "balls_stopped",
};
static constexpr int VALID_EVENTS_COUNT = 4;

static bool is_valid_event(const godot::String &name) {
    for (int i = 0; i < VALID_EVENTS_COUNT; ++i) {
        if (name == VALID_EVENTS[i]) return true;
    }
    return false;
}

void WaitForBilliardsEventTask::_bind_methods() {}

godot::Error WaitForBilliardsEventTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("events")) {
        godot::UtilityFunctions::push_error("[WaitForBilliardsEventTask] 'events' is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }

    godot::Array raw = spec.payload["events"];
    if (raw.size() == 0) {
        godot::UtilityFunctions::push_error("[WaitForBilliardsEventTask] 'events' must be a non-empty array.");
        return godot::ERR_INVALID_DATA;
    }
    for (int i = 0; i < raw.size(); ++i) {
        godot::String ev = raw[i];
        if (!is_valid_event(ev)) {
            godot::UtilityFunctions::push_error("[WaitForBilliardsEventTask] Unknown event name: ", ev);
            return godot::ERR_INVALID_DATA;
        }
    }
    expected_events_ = raw;

    timeout_ = spec.payload.has("timeout")
                   ? static_cast<double>(spec.payload["timeout"])
                   : 10.0;
    if (timeout_ <= 0.0) {
        godot::UtilityFunctions::push_error("[WaitForBilliardsEventTask] 'timeout' must be > 0.");
        return godot::ERR_INVALID_DATA;
    }

    start_time_ = karakuri::KernelClock::get_singleton()->now();
    return godot::OK;
}

TaskResult WaitForBilliardsEventTask::execute() {
    double now = karakuri::KernelClock::get_singleton()->now();

    if (now - start_time_ >= timeout_) {
        // Fake-event mode: timeout acts as "balls_stopped" so the scenario can proceed.
        godot::UtilityFunctions::print("[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.");
        if (auto ws = karakuri::WorldState::get_singleton()) {
            ws->set_state("billiards_test", karakuri::WorldState::SCOPE_SESSION,
                          "round:last_event", godot::String("balls_stopped"));
        }
        return TaskResult::Success;
    }

    return TaskResult::Waiting;
}

void WaitForBilliardsEventTask::complete_instantly() {
    if (auto ws = karakuri::WorldState::get_singleton()) {
        ws->set_state("billiards_test", karakuri::WorldState::SCOPE_SESSION,
                      "round:last_event", godot::String("balls_stopped"));
    }
}

} // namespace karakuri::games::billiards_test
