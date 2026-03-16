#include "mystery_test_game.h"

#include "core/action_registry.h"
#include "core/scenario/scenario_runner.h"
#include "tasks/show_dialogue_task.h"
#include "tasks/add_evidence_task.h"
#include "tasks/check_evidence_task.h"
#include "tasks/end_game_task.h"
#include <godot_cpp/classes/engine.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri {

void MysteryTestGame::_bind_methods() {
}

void MysteryTestGame::_ready() {
    godot::UtilityFunctions::print("[MysteryTestGame] Bootstrapping...");

    if (auto reg = ActionRegistry::get_singleton()) {
        reg->register_action_class<ShowDialogueTask>("show_dialogue");
        reg->register_action_class<AddEvidenceTask>("add_evidence");
        reg->register_action_class<CheckEvidenceTask>("check_evidence");
        reg->register_action_class<EndGameTask>("end_game");
        godot::UtilityFunctions::print("[MysteryTestGame] Registered minimal tasks.");
    }
}

} // namespace karakuri
