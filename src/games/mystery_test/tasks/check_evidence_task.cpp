#include "check_evidence_task.h"

#include "core/world_state.h"
#include "core/scenario/scenario_runner.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri {

void CheckEvidenceTask::_bind_methods() {}

godot::Error CheckEvidenceTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("id") || !spec.payload.has("if_true") || !spec.payload.has("if_false")) {
        godot::UtilityFunctions::push_error("[CheckEvidenceTask] Missing parameters in payload.");
        return godot::ERR_INVALID_DATA;
    }
    evidence_id_ = spec.payload["id"];
    if_true_ = spec.payload["if_true"];
    if_false_ = spec.payload["if_false"];
    return godot::OK;
}

TaskResult CheckEvidenceTask::execute() {
    bool found = false;
    if (auto ws = WorldState::get_singleton()) {
        found = ws->get_state("mystery_test", WorldState::SCOPE_GLOBAL, "evidence:" + evidence_id_, false);
    }

    godot::String target = found ? if_true_ : if_false_;
    godot::UtilityFunctions::print("[CheckEvidenceTask] Evidence '", evidence_id_, "' found: ", found, ". Jumping to: ", target);

    if (runner_) {
        runner_->load_scene_by_id(target);
    }

    return TaskResult::Success;
}

} // namespace karakuri
