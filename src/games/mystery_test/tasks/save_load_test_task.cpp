#include "save_load_test_task.h"
#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::mystery_test {

void SaveLoadTestTask::_bind_methods() {}

godot::Error SaveLoadTestTask::validate_and_setup(const TaskSpec &spec) {
    if (spec.payload.has("key")) {
        test_key_ = spec.payload["key"];
    }
    return godot::OK;
}

TaskResult SaveLoadTestTask::execute() {
    auto ws = WorldState::get_singleton();
    if (!ws) return TaskResult::Failed;

    // 1. Mutation
    ws->set_state("mystery_test", WorldState::SCOPE_GLOBAL, test_key_, true);
    godot::UtilityFunctions::print("[SaveLoadTestTask] Set flag: ", test_key_);

    // 2. Simulate Save
    godot::Dictionary saved_data = ws->serialize_globals();
    godot::UtilityFunctions::print("[SaveLoadTestTask] Serialized Globals.");

    // 3. Clear and Simulate Reload
    ws->clear_scope(WorldState::SCOPE_GLOBAL);
    if (ws->get_state("mystery_test", WorldState::SCOPE_GLOBAL, test_key_, false)) {
        godot::UtilityFunctions::push_error("[SaveLoadTestTask] Failed to clear scope for test.");
        return TaskResult::Failed;
    }

    ws->deserialize_globals(saved_data);
    godot::UtilityFunctions::print("[SaveLoadTestTask] Deserialized Globals.");

    // 4. Verify Persistence
    bool value = ws->get_state("mystery_test", WorldState::SCOPE_GLOBAL, test_key_, false);
    if (value) {
        godot::UtilityFunctions::print("[SaveLoadTestTask] SUCCESS: Persistence verified for ", test_key_);
        return TaskResult::Success;
    } else {
        godot::UtilityFunctions::push_error("[SaveLoadTestTask] FAILED: Persistence check failed for ", test_key_);
        return TaskResult::Failed;
    }
}

} // namespace karakuri::games::mystery_test
