#include "end_game_task.h"

#include "core/scenario/scenario_runner.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri {

void EndGameTask::_bind_methods() {
    ADD_SIGNAL(godot::MethodInfo("game_ended", godot::PropertyInfo(godot::Variant::STRING, "result")));
}

godot::Error EndGameTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("result")) {
        godot::UtilityFunctions::push_error("[EndGameTask] 'result' is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }
    result_ = spec.payload["result"];
    return godot::OK;
}

TaskResult EndGameTask::execute() {
    if (runner_) {
        runner_->emit_signal("game_ended", result_);
        godot::UtilityFunctions::print("[EndGameTask] Game ended with result: ", result_);
    }
    return TaskResult::Success;
}

} // namespace karakuri
