#include "discover_evidence_task.h"

#include "core/world_state.h"
#include "core/scenario/scenario_runner.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::mystery_test {

void DiscoverEvidenceTask::_bind_methods() {}

godot::Error DiscoverEvidenceTask::validate_and_setup(const TaskSpec &spec) {
    // Accept "evidence_id" (canonical) or legacy "id".
    if (spec.payload.has("evidence_id")) {
        evidence_id_ = spec.payload["evidence_id"];
    } else if (spec.payload.has("id")) {
        evidence_id_ = spec.payload["id"];
    } else {
        godot::UtilityFunctions::push_error(
                "[DiscoverEvidenceTask] Required field 'evidence_id' is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }

    if (!spec.payload.has("location")) {
        godot::UtilityFunctions::push_error(
                "[DiscoverEvidenceTask] Required field 'location' is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }
    location_ = spec.payload["location"];
    return godot::OK;
}

TaskResult DiscoverEvidenceTask::execute() {
    WorldState *ws = WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error("[DiscoverEvidenceTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    const godot::String key = "evidence:" + evidence_id_;

    // Avoid duplicate writes: skip silently if already present in SESSION scope.
    if (ws->has_flag("mystery_test", WorldState::SCOPE_SESSION, key)) {
        godot::UtilityFunctions::print(
                "[DiscoverEvidenceTask] Evidence already discovered (skipped): ", evidence_id_);
        return TaskResult::Success;
    }

    ws->set_state("mystery_test", WorldState::SCOPE_SESSION, key, true);
    godot::UtilityFunctions::print(
            "[DiscoverEvidenceTask] Evidence discovered: ", evidence_id_, " (", location_, ")");
    return TaskResult::Success;
}

} // namespace karakuri::games::mystery_test
