#include "resolve_rhythm_progress_task.h"

#include "core/world_state.h"

namespace karakuri::games::rhythm_test {

namespace {

constexpr int kScopeSession = karakuri::WorldState::SCOPE_SESSION;

} // namespace

void ResolveRhythmProgressTask::_bind_methods() {}

godot::Error ResolveRhythmProgressTask::validate_and_setup(const TaskSpec & /*spec*/) {
    return godot::OK;
}

TaskResult ResolveRhythmProgressTask::execute() {
    auto *ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error(
            "[ResolveRhythmProgressTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const godot::String result = ws->get_state("rhythm_test", kScopeSession, "judge:last_result", godot::String("none"));
    if (result == "none") {
        return TaskResult::Success;
    }

    int chart_index = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "chart:index", 0));
    const int note_count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "chart:note_count", 0));
    if (chart_index >= note_count) {
        return TaskResult::Success;
    }

    if (result == "perfect") {
        const int count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "judge:perfect_count", 0));
        ws->set_state("rhythm_test", kScopeSession, "judge:perfect_count", count + 1);
    } else if (result == "good") {
        const int count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "judge:good_count", 0));
        ws->set_state("rhythm_test", kScopeSession, "judge:good_count", count + 1);
    } else if (result == "miss") {
        const int count = static_cast<int>(ws->get_state("rhythm_test", kScopeSession, "judge:miss_count", 0));
        ws->set_state("rhythm_test", kScopeSession, "judge:miss_count", count + 1);
    }

    chart_index += 1;
    ws->set_state("rhythm_test", kScopeSession, "chart:index", chart_index);
    ws->set_state("rhythm_test", kScopeSession, "tap:last_time_ms", -1);
    return TaskResult::Success;
}

} // namespace karakuri::games::rhythm_test