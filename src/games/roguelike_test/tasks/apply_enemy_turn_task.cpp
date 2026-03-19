#include "apply_enemy_turn_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::roguelike_test {

static constexpr int MAP_SIZE_E = 5;

void ApplyEnemyTurnTask::_bind_methods() {}

godot::Error ApplyEnemyTurnTask::validate_and_setup(const TaskSpec & /*spec*/) {
    return godot::OK;
}

TaskResult ApplyEnemyTurnTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error(
            "[ApplyEnemyTurnTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;

    int e_hp = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:hp", 0));
    if (e_hp <= 0) {
        // Enemy already defeated — skip turn silently.
        godot::UtilityFunctions::print("[ApplyEnemyTurnTask] enemy_1 is defeated, skipping.");
        return TaskResult::Success;
    }

    int ex = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:x", 3));
    int ey = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:y", 3));
    int px = static_cast<int>(ws->get_state("roguelike_test", S, "player:x", 1));
    int py = static_cast<int>(ws->get_state("roguelike_test", S, "player:y", 1));
    int p_hp = static_cast<int>(ws->get_state("roguelike_test", S, "player:hp", 3));

    // Manhattan distance
    int dx = px - ex;
    int dy = py - ey;
    int dist = (dx >= 0 ? dx : -dx) + (dy >= 0 ? dy : -dy);

    if (dist == 1) {
        // Attack the player
        int new_p_hp = p_hp - 1;
        ws->set_state("roguelike_test", S, "player:hp",        new_p_hp);
        ws->set_state("roguelike_test", S, "last_action:actor", godot::String("enemy_1"));
        ws->set_state("roguelike_test", S, "last_action:type",  godot::String("attack"));

        godot::UtilityFunctions::print(
            "[ApplyEnemyTurnTask] enemy_1 attacked player: hp ", p_hp, " → ", new_p_hp);

        if (new_p_hp <= 0) {
            ws->set_state("roguelike_test", S, "round:result", godot::String("fail"));
            godot::UtilityFunctions::print("[ApplyEnemyTurnTask] Player defeated → fail.");
        }
    } else {
        // Move one step toward the player along the dominant axis.
        int nx = ex;
        int ny = ey;

        if ((dx >= 0 ? dx : -dx) >= (dy >= 0 ? dy : -dy)) {
            nx += (dx > 0 ? 1 : -1);
        } else {
            ny += (dy > 0 ? 1 : -1);
        }

        // Clamp to map bounds (safety — positions are always valid, but guard anyway)
        if (nx < 0) nx = 0;
        if (nx >= MAP_SIZE_E) nx = MAP_SIZE_E - 1;
        if (ny < 0) ny = 0;
        if (ny >= MAP_SIZE_E) ny = MAP_SIZE_E - 1;

        // Do not move on top of player
        if (nx == px && ny == py) {
            // Blocked by player — stay in place
            nx = ex;
            ny = ey;
        }

        ws->set_state("roguelike_test", S, "enemy_1:x",        nx);
        ws->set_state("roguelike_test", S, "enemy_1:y",        ny);
        ws->set_state("roguelike_test", S, "last_action:actor", godot::String("enemy_1"));
        ws->set_state("roguelike_test", S, "last_action:type",  godot::String("move"));

        godot::UtilityFunctions::print(
            "[ApplyEnemyTurnTask] enemy_1 moved: (", ex, ",", ey, ") → (", nx, ",", ny, ")");
    }

    return TaskResult::Success;
}

} // namespace karakuri::games::roguelike_test
