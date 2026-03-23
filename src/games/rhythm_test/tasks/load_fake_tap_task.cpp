#include "load_fake_tap_task.h"

#include "core/kernel_clock.h"
#include "core/world_state.h"

namespace karakuri::games::rhythm_test {

namespace {

constexpr int kScopeSession = karakuri::WorldState::SCOPE_SESSION;

godot::String tap_time_key(int index) {
    return godot::String("chart:tap_") + godot::String::num_int64(index) + ":time_ms";
}

} // namespace

void LoadFakeTapTask::_bind_methods() {}

godot::Error LoadFakeTapTask::validate_and_setup(const TaskSpec & /*spec*/) {
    return godot::OK;
}

TaskResult LoadFakeTapTask::execute() {
    auto *ws = karakuri::WorldState::get_singleton();
    auto *clock = karakuri::KernelClock::get_singleton();
    if (!ws || !clock) {
        godot::UtilityFunctions::push_error(
            "[LoadFakeTapTask] WorldState or KernelClock singleton is null.");
        return TaskResult::Failed;
    }

    const int chart_index = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "chart:index", 0));
    const int note_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "chart:note_count", 0));
    const int now_ms = static_cast<int>(clock->now() * 1000.0);
    ws->set_state("rhythm_test", kScopeSession, "clock:now_ms", now_ms);

    if (chart_index >= note_count) {
        ws->set_state("rhythm_test", kScopeSession, "tap:last_time_ms", -1);
        return TaskResult::Success;
    }

    const int scheduled_tap = static_cast<int>(ws->get_state(
        "rhythm_test", kScopeSession, tap_time_key(chart_index), -1));
    const int effective_tap = (scheduled_tap >= 0 && scheduled_tap <= now_ms) ? scheduled_tap : -1;
    ws->set_state("rhythm_test", kScopeSession, "tap:last_time_ms", effective_tap);
    return TaskResult::Success;
}

} // namespace karakuri::games::rhythm_test