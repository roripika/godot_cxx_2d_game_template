#include "resolve_roguelike_turn_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::roguelike_test {

void ResolveRoguelikeTurnTask::_bind_methods() {}

godot::Error ResolveRoguelikeTurnTask::validate_and_setup(const TaskSpec & /*spec*/) {
    return godot::OK;
}

TaskResult ResolveRoguelikeTurnTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error(
            "[ResolveRoguelikeTurnTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;

    int turn = static_cast<int>(ws->get_state("roguelike_test", S, "turn:index", 0));
    int next_turn = turn + 1;
    ws->set_state("roguelike_test", S, "turn:index",        next_turn);
    ws->set_state("roguelike_test", S, "turn:phase",        godot::String("player"));
    ws->set_state("roguelike_test", S, "last_action:actor", godot::String(""));
    ws->set_state("roguelike_test", S, "last_action:type",  godot::String(""));

    godot::UtilityFunctions::print(
        "[ResolveRoguelikeTurnTask] Turn resolved: index ", turn, " → ", next_turn);

    return TaskResult::Success;
}

} // namespace karakuri::games::roguelike_test
