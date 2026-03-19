#include "roguelike_test_game.h"

#include "core/action_registry.h"
#include "tasks/setup_roguelike_round_task.h"
#include "tasks/load_fake_player_command_task.h"
#include "tasks/apply_player_move_task.h"
#include "tasks/apply_player_attack_task.h"
#include "tasks/apply_enemy_turn_task.h"
#include "tasks/resolve_roguelike_turn_task.h"
#include "tasks/evaluate_roguelike_round_task.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::roguelike_test {

void RoguelikeTestGame::_bind_methods() {}

void RoguelikeTestGame::_ready() {
    godot::UtilityFunctions::print("[RoguelikeTestGame] Bootstrapping...");

    if (auto reg = karakuri::ActionRegistry::get_singleton()) {
        reg->register_action_class<SetupRoguelikeRoundTask>("setup_roguelike_round");
        reg->register_action_class<LoadFakePlayerCommandTask>("load_fake_player_command");
        reg->register_action_class<ApplyPlayerMoveTask>("apply_player_move");
        reg->register_action_class<ApplyPlayerAttackTask>("apply_player_attack");
        reg->register_action_class<ApplyEnemyTurnTask>("apply_enemy_turn");
        reg->register_action_class<ResolveRoguelikeTurnTask>("resolve_roguelike_turn");
        reg->register_action_class<EvaluateRoguelikeRoundTask>("evaluate_roguelike_round");

        godot::UtilityFunctions::print("[RoguelikeTestGame] Registered 7 roguelike test tasks.");
    }
}

} // namespace karakuri::games::roguelike_test
