#include "add_evidence_task.h"

#include "core/world_state.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri {

void AddEvidenceTask::_bind_methods() {}

godot::Error AddEvidenceTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("id")) {
        godot::UtilityFunctions::push_error("[AddEvidenceTask] 'id' is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }
    evidence_id_ = spec.payload["id"];
    return godot::OK;
}

TaskResult AddEvidenceTask::execute() {
    if (auto ws = WorldState::get_singleton()) {
        ws->set_state("mystery_test", WorldState::SCOPE_GLOBAL, "evidence:" + evidence_id_, true);
        godot::UtilityFunctions::print("[AddEvidenceTask] Added evidence: ", evidence_id_);
    }
    return TaskResult::Success;
}

} // namespace karakuri
