#include "apply_player_move_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::roguelike_test {

// 5x5 map: valid x/y range is [0, 4].
static constexpr int MAP_SIZE = 5;

void ApplyPlayerMoveTask::_bind_methods() {}

godot::Error ApplyPlayerMoveTask::validate_and_setup(const TaskSpec &/*spec*/) {
    return godot::OK;
}

TaskResult ApplyPlayerMoveTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerMoveTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;

    godot::String cmd = ws->get_state("roguelike_test", S, "last_action:type", godot::String(""));
    int px = static_cast<int>(ws->get_state("roguelike_test", S, "player:x", 0));
    int py = static_cast<int>(ws->get_state("roguelike_test", S, "player:y", 0));

    int dx = 0, dy = 0;
    if      (cmd == "move_up")    { dy = -1; }
    else if (cmd == "move_down")  { dy =  1; }
    else if (cmd == "move_left")  { dx = -1; }
    else if (cmd == "move_right") { dx =  1; }
    else {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerMoveTask] last_action:type is not a move command: ", cmd);
        return TaskResult::Failed;
    }

    int nx = px + dx;
    int ny = py + dy;

    // Boundary check
    if (nx < 0 || nx >= MAP_SIZE || ny < 0 || ny >= MAP_SIZE) {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerMoveTask] Move out of bounds: (", nx, ",", ny, ").");
        return TaskResult::Failed;
    }

    // Occupancy check: enemy_1
    int ex = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:x", -1));
    int ey = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:y", -1));
    int e_hp = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:hp", 0));
    if (e_hp > 0 && nx == ex && ny == ey) {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerMoveTask] Cell (", nx, ",", ny, ") is occupied by enemy_1.");
        return TaskResult::Failed;
    }

    ws->set_state("roguelike_test", S, "player:x", nx);
    ws->set_state("roguelike_test", S, "player:y", ny);

    godot::UtilityFunctions::print(
        "[ApplyPlayerMoveTask] Player moved to (", nx, ",", ny, ").");

    // Goal check
    int gx = static_cast<int>(ws->get_state("roguelike_test", S, "goal:x", -1));
    int gy = static_cast<int>(ws->get_state("roguelike_test", S, "goal:y", -1));
    if (nx == gx && ny == gy) {
        ws->set_state("roguelike_test", S, "round:result", godot::String("clear"));
        godot::UtilityFunctions::print("[ApplyPlayerMoveTask] Player reached goal → clear.");
    }

    return TaskResult::Success;
}

} // namespace karakuri::games::roguelike_test
