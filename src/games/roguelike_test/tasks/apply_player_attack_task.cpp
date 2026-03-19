#include "apply_player_attack_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::roguelike_test {

void ApplyPlayerAttackTask::_bind_methods() {}

godot::Error ApplyPlayerAttackTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("target")) {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerAttackTask] 'target' is required in payload.");
        return godot::ERR_INVALID_DATA;
    }
    const godot::String t = spec.payload["target"];
    if (t != "enemy_1") {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerAttackTask] Unknown target: ", t,
            ". Only 'enemy_1' is supported.");
        return godot::ERR_INVALID_DATA;
    }
    target_ = t;
    return godot::OK;
}

TaskResult ApplyPlayerAttackTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerAttackTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;

    int px = static_cast<int>(ws->get_state("roguelike_test", S, "player:x", 0));
    int py = static_cast<int>(ws->get_state("roguelike_test", S, "player:y", 0));
    int ex = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:x", -1));
    int ey = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:y", -1));
    int e_hp = static_cast<int>(ws->get_state("roguelike_test", S, "enemy_1:hp", 0));

    if (e_hp <= 0) {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerAttackTask] enemy_1 is already defeated.");
        return TaskResult::Failed;
    }

    // Adjacency check (Manhattan distance == 1)
    int dist = (px > ex ? px - ex : ex - px) + (py > ey ? py - ey : ey - py);
    if (dist != 1) {
        godot::UtilityFunctions::push_error(
            "[ApplyPlayerAttackTask] enemy_1 is not adjacent (dist=", dist, ").");
        return TaskResult::Failed;
    }

    int new_hp = e_hp - 1;
    ws->set_state("roguelike_test", S, "enemy_1:hp",       new_hp);
    ws->set_state("roguelike_test", S, "last_action:type", godot::String("attack"));
    ws->set_state("roguelike_test", S, "last_action:actor", godot::String("player"));

    godot::UtilityFunctions::print(
        "[ApplyPlayerAttackTask] Player attacked enemy_1: hp ", e_hp, " → ", new_hp);

    if (new_hp <= 0) {
        ws->set_state("roguelike_test", S, "round:result", godot::String("clear"));
        godot::UtilityFunctions::print("[ApplyPlayerAttackTask] enemy_1 defeated → clear.");
    }

    return TaskResult::Success;
}

} // namespace karakuri::games::roguelike_test
