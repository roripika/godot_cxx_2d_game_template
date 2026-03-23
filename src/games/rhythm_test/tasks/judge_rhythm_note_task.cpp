#include "judge_rhythm_note_task.h"

#include "core/kernel_clock.h"
#include "core/world_state.h"

namespace karakuri::games::rhythm_test {

namespace {

constexpr int kScopeSession = karakuri::WorldState::SCOPE_SESSION;

godot::String note_time_key(int index) {
    return godot::String("chart:note_") + godot::String::num_int64(index) + ":time_ms";
}

int abs_int(int value) {
    return value >= 0 ? value : -value;
}

} // namespace

void JudgeRhythmNoteTask::_bind_methods() {}

godot::Error JudgeRhythmNoteTask::validate_and_setup(const TaskSpec & /*spec*/) {
    return godot::OK;
}

TaskResult JudgeRhythmNoteTask::execute() {
    auto *ws = karakuri::WorldState::get_singleton();
    auto *clock = karakuri::KernelClock::get_singleton();
    if (!ws || !clock) {
        godot::UtilityFunctions::push_error(
            "[JudgeRhythmNoteTask] WorldState or KernelClock singleton is null.");
        return TaskResult::Failed;
    }

    const int chart_index = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "chart:index", 0));
    const int note_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "chart:note_count", 0));
    if (chart_index >= note_count) {
        ws->set_state("rhythm_test", kScopeSession, "judge:last_result", godot::String("none"));
        return TaskResult::Success;
    }

    const int now_ms = static_cast<int>(clock->now() * 1000.0);
    const int note_time = static_cast<int>(ws->get_state(
        "rhythm_test", kScopeSession, note_time_key(chart_index), -1));
    const int tap_time = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "tap:last_time_ms", -1));
    const int perfect_window_ms = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "config:perfect_window_ms", 50));
    const int good_window_ms = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "config:good_window_ms", 150));

    godot::String result = "none";
    if (tap_time >= 0) {
        const int diff_ms = abs_int(tap_time - note_time);
        if (diff_ms <= perfect_window_ms) {
            result = "perfect";
        } else if (diff_ms <= good_window_ms) {
            result = "good";
        } else {
            result = "miss";
        }
    } else if (now_ms > note_time + good_window_ms) {
        result = "miss";
    }

    ws->set_state("rhythm_test", kScopeSession, "judge:last_result", result);
    ws->set_state("rhythm_test", kScopeSession, "clock:now_ms", now_ms);
    return TaskResult::Success;
}

} // namespace karakuri::games::rhythm_test