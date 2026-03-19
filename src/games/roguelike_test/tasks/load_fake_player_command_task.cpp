#include "load_fake_player_command_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::roguelike_test {

static const char* VALID_COMMANDS[] = {
    "move_up", "move_down", "move_left", "move_right", "attack"
};
static constexpr int VALID_COMMANDS_COUNT = 5;

static bool is_valid_command(const godot::String &cmd) {
    for (int i = 0; i < VALID_COMMANDS_COUNT; ++i) {
        if (cmd == VALID_COMMANDS[i]) return true;
    }
    return false;
}

void LoadFakePlayerCommandTask::_bind_methods() {}

godot::Error LoadFakePlayerCommandTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("command")) {
        godot::UtilityFunctions::push_error(
            "[LoadFakePlayerCommandTask] 'command' is required in payload.");
        return godot::ERR_INVALID_DATA;
    }
    const godot::String cmd = spec.payload["command"];
    if (!is_valid_command(cmd)) {
        godot::UtilityFunctions::push_error(
            "[LoadFakePlayerCommandTask] Unknown command: ", cmd);
        return godot::ERR_INVALID_DATA;
    }
    command_ = cmd;
    return godot::OK;
}

TaskResult LoadFakePlayerCommandTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error(
            "[LoadFakePlayerCommandTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }
    const int S = karakuri::WorldState::SCOPE_SESSION;
    ws->set_state("roguelike_test", S, "last_action:type",  command_);
    ws->set_state("roguelike_test", S, "last_action:actor", godot::String("player"));
    ws->set_state("roguelike_test", S, "turn:phase",        godot::String("player_act"));

    godot::UtilityFunctions::print(
        "[LoadFakePlayerCommandTask] Loaded command: ", command_);
    return TaskResult::Success;
}

} // namespace karakuri::games::roguelike_test
