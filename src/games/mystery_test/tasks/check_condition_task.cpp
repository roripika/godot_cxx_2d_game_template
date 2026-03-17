#include "check_condition_task.h"

#include "core/world_state.h"
#include "core/scenario/scenario_runner.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::mystery_test {

void CheckConditionTask::_bind_methods() {}

godot::Error CheckConditionTask::validate_and_setup(const TaskSpec &spec) {
    const bool has_all = spec.payload.has("all_of");
    const bool has_any = spec.payload.has("any_of");

    // Exactly one of all_of / any_of must be present.
    if (has_all == has_any) {
        godot::UtilityFunctions::push_error(
                "[CheckConditionTask] Payload must contain exactly one of 'all_of' or 'any_of'.");
        return godot::ERR_INVALID_DATA;
    }
    if (!spec.payload.has("if_true") || !spec.payload.has("if_false")) {
        godot::UtilityFunctions::push_error(
                "[CheckConditionTask] 'if_true' and 'if_false' are required in payload.");
        return godot::ERR_INVALID_DATA;
    }

    godot::Array raw = has_all ? godot::Array(spec.payload["all_of"])
                               : godot::Array(spec.payload["any_of"]);

    // Validate each condition entry.
    for (int i = 0; i < raw.size(); ++i) {
        if (raw[i].get_type() != godot::Variant::DICTIONARY) {
            godot::UtilityFunctions::push_error(
                    "[CheckConditionTask] Each condition entry must be a Dictionary.");
            return godot::ERR_INVALID_DATA;
        }
        godot::Dictionary entry = raw[i];
        if (!entry.has("evidence")) {
            godot::UtilityFunctions::push_error(
                    "[CheckConditionTask] Each condition entry must contain key 'evidence'.");
            return godot::ERR_INVALID_DATA;
        }
    }

    conditions_ = raw;
    use_all_of_ = has_all;
    if_true_    = spec.payload["if_true"];
    if_false_   = spec.payload["if_false"];
    return godot::OK;
}

TaskResult CheckConditionTask::execute() {
    WorldState *ws = WorldState::get_singleton();
    if (!ws) {
        godot::UtilityFunctions::push_error("[CheckConditionTask] WorldState singleton is null.");
        return TaskResult::Failed;
    }

    bool result;
    if (use_all_of_) {
        // all_of: every listed evidence must be true.
        result = true;
        for (int i = 0; i < conditions_.size(); ++i) {
            const godot::String id = godot::Dictionary(conditions_[i])["evidence"];
            const godot::Variant val = ws->get_state(
                    "mystery_test", WorldState::SCOPE_SESSION, "evidence:" + id, false);
            if (!(bool)val) {
                result = false;
                break;
            }
        }
    } else {
        // any_of: at least one listed evidence must be true.
        result = false;
        for (int i = 0; i < conditions_.size(); ++i) {
            const godot::String id = godot::Dictionary(conditions_[i])["evidence"];
            const godot::Variant val = ws->get_state(
                    "mystery_test", WorldState::SCOPE_SESSION, "evidence:" + id, false);
            if ((bool)val) {
                result = true;
                break;
            }
        }
    }

    const godot::String mode_str = use_all_of_ ? "all_of" : "any_of";
    const godot::String target   = result ? if_true_ : if_false_;
    godot::UtilityFunctions::print(
            "CheckConditionTask: ", mode_str, " evaluated ",
            result ? "true" : "false", " -> ", target);

    if (runner_) {
        runner_->load_scene_by_id(target);
    }
