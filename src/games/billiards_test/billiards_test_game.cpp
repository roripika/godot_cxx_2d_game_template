#include "billiards_test_game.h"

#include "core/action_registry.h"
#include "tasks/setup_billiards_round_task.h"
#include "tasks/wait_for_billiards_event_task.h"
#include "tasks/record_billiards_event_task.h"
#include "tasks/evaluate_billiards_round_task.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::billiards_test {

void BilliardsTestGame::_bind_methods() {}

void BilliardsTestGame::_ready() {
    godot::UtilityFunctions::print("[BilliardsTestGame] Bootstrapping...");

    if (auto reg = karakuri::ActionRegistry::get_singleton()) {
        reg->register_action_class<SetupBilliardsRoundTask>("setup_billiards_round");
        reg->register_action_class<WaitForBilliardsEventTask>("wait_for_billiards_event");
        reg->register_action_class<RecordBilliardsEventTask>("record_billiards_event");
        reg->register_action_class<EvaluateBilliardsRoundTask>("evaluate_billiards_round");

        godot::UtilityFunctions::print("[BilliardsTestGame] Registered 4 billiards test tasks.");
    }
}

} // namespace karakuri::games::billiards_test
