#include "mystery_test_game.h"

#include "core/action_registry.h"
#include "core/scenario/scenario_runner.h"
#include "tasks/show_dialogue_task.h"
#include "tasks/add_evidence_task.h"
#include "tasks/check_evidence_task.h"
#include "tasks/end_game_task.h"
#include "tasks/check_condition_task.h"
#include "tasks/discover_evidence_task.h"
#include "tasks/wait_for_signal_task.h"
#include "tasks/parallel_task_group.h"
#include "tasks/save_load_test_task.h"
#include <godot_cpp/classes/engine.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri {

using namespace karakuri::games::mystery_test;

void MysteryTestGame::_bind_methods() {
}

void MysteryTestGame::_ready() {
    godot::UtilityFunctions::print("[MysteryTestGame] Bootstrapping...");

    if (auto reg = ActionRegistry::get_singleton()) {
        reg->register_action_class<ShowDialogueTask>("show_dialogue");
        reg->register_action_class<AddEvidenceTask>("add_evidence");
        reg->register_action_class<CheckEvidenceTask>("check_evidence");
        reg->register_action_class<CheckConditionTask>("check_condition");
        reg->register_action_class<DiscoverEvidenceTask>("discover_evidence");
        reg->register_action_class<WaitForSignalTask>("wait_for_signal");
        reg->register_action_class<ParallelTaskGroup>("parallel");
        reg->register_action_class<SaveLoadTestTask>("save_load_test");
        reg->register_action_class<EndGameTask>("end_game");
        
        godot::UtilityFunctions::print("[MysteryTestGame] Registered ultimate fitness test tasks.");
    }
}

} // namespace karakuri
