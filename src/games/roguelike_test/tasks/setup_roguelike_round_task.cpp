#include "setup_roguelike_round_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::roguelike_test {

void SetupRoguelikeRoundTask::_bind_methods() {}

static bool is_valid_coord(int v) { return v >= 0 && v < 5; }

godot::Error SetupRoguelikeRoundTask::validate_and_setup(const TaskSpec &spec) {
    if (spec.payload.has("player_hp")) {
        int v = static_cast<int>(spec.payload["player_hp"]);
        if (v <= 0) {
            godot::UtilityFunctions::push_error(
                "[SetupRoguelikeRoundTask] 'player_hp' must be > 0.");
            return godot::ERR_INVALID_DATA;
        }
        player_hp_ = v;
    }
    if (spec.payload.has("enemy_hp")) {
        int v = static_cast<int>(spec.payload["enemy_hp"]);
        if (v <= 0) {
            godot::UtilityFunctions::push_error(
                "[SetupRoguelikeRoundTask] 'enemy_hp' must be > 0.");
            return godot::ERR_INVALID_DATA;
        }
        enemy_hp_ = v;
    }
    if (spec.payload.has("player_x")) {
        int v = static_cast<int>(spec.payload["player_x"]);
        if (!is_valid_coord(v)) {
            godot::UtilityFunctions::push_error(
                "[SetupRoguelikeRoundTask] 'player_x' out of range [0,4]: ", v);
            return godot::ERR_INVALID_DATA;
        }
        player_x_ = v;
    }
    if (spec.payload.has("player_y")) {
        int v = static_cast<int>(spec.payload["player_y"]);
        if (!is_valid_coord(v)) {
            godot::UtilityFunctions::push_error(
                "[SetupRoguelikeRoundTask] 'player_y' out of range [0,4]: ", v);
            return godot::ERR_INVALID_DATA;
        }
        player_y_ = v;
    }
    if (spec.payload.has("enemy_1_x")) {
        int v = static_cast<int>(spec.payload["enemy_1_x"]);
        if (!is_valid_coord(v)) {
            godot::UtilityFunctions::push_error(
                "[SetupRoguelikeRoundTask] 'enemy_1_x' out of range [0,4]: ", v);
            return godot::ERR_INVALID_DATA;
        }
        enemy_1_x_ = v;
    }
    if (spec.payload.has("enemy_1_y")) {
        int v = static_cast<int>(spec.payload["enemy_1_y"]);
        if (!is_valid_coord(v)) {
            godot::UtilityFunctions::push_error(
                "[SetupRoguelikeRoundTask] 'enemy_1_y' out of range [0,4]: ", v);
            return godot::ERR_INVALID_DATA;
        }
        enemy_1_y_ = v;
    }
    return godot::OK;
}

TaskResult SetupRoguelikeRoundTask::execute() {
    auto ws = karakuri::WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error(
            "[SetupRoguelikeRoundTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const int S = karakuri::WorldState::SCOPE_SESSION;
    // Turn state
    ws->set_state("roguelike_test", S, "turn:index",        0);
    ws->set_state("roguelike_test", S, "turn:phase",        godot::String("player"));
    // Player
    ws->set_state("roguelike_test", S, "player:x",          player_x_);
    ws->set_state("roguelike_test", S, "player:y",          player_y_);
    ws->set_state("roguelike_test", S, "player:hp",         player_hp_);
    // Enemy (1 only in initial version)
    ws->set_state("roguelike_test", S, "enemy_1:x",         enemy_1_x_);
    ws->set_state("roguelike_test", S, "enemy_1:y",         enemy_1_y_);
    ws->set_state("roguelike_test", S, "enemy_1:hp",        enemy_hp_);
    // Goal
    ws->set_state("roguelike_test", S, "goal:x",            4);
    ws->set_state("roguelike_test", S, "goal:y",            4);
    // Result
    ws->set_state("roguelike_test", S, "round:result",      godot::String(""));
    ws->set_state("roguelike_test", S, "last_action:actor", godot::String(""));
    ws->set_state("roguelike_test", S, "last_action:type",  godot::String(""));

    godot::UtilityFunctions::print(
        "[SetupRoguelikeRoundTask] Round initialised: player_hp=", player_hp_,
        " enemy_hp=", enemy_hp_);
    return TaskResult::Success;
}

} // namespace karakuri::games::roguelike_test
