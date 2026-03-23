#include "advance_rhythm_clock_task.h"

#include "core/kernel_clock.h"
#include "core/world_state.h"

namespace karakuri::games::rhythm_test {

namespace {

constexpr int kScopeSession = karakuri::WorldState::SCOPE_SESSION;

} // namespace

void AdvanceRhythmClockTask::_bind_methods() {}

godot::Error AdvanceRhythmClockTask::validate_and_setup(const TaskSpec & /*spec*/) {
    return godot::OK;
}

TaskResult AdvanceRhythmClockTask::execute() {
    auto *ws = karakuri::WorldState::get_singleton();
    auto *clock = karakuri::KernelClock::get_singleton();
    if (!ws || !clock) {
        godot::UtilityFunctions::push_error(
            "[AdvanceRhythmClockTask] WorldState or KernelClock singleton is null.");
        return TaskResult::Failed;
    }

    const int advance_ms = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "config:advance_ms", 1000));
    if (advance_ms <= 0) {
        godot::UtilityFunctions::push_error(
            "[AdvanceRhythmClockTask] config:advance_ms must be > 0.");
        return TaskResult::Failed;
    }

    clock->advance(static_cast<double>(advance_ms) / 1000.0);
    ws->set_state("rhythm_test", kScopeSession, "clock:now_ms", static_cast<int>(clock->now() * 1000.0));
    return TaskResult::Success;
}

} // namespace karakuri::games::rhythm_test