#include "show_dialogue_task.h"

#include "core/scenario/scenario_runner.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri {

void ShowDialogueTask::_bind_methods() {
    ADD_SIGNAL(godot::MethodInfo("dialogue_requested", godot::PropertyInfo(godot::Variant::STRING, "speaker"), godot::PropertyInfo(godot::Variant::STRING, "text")));
}

godot::Error ShowDialogueTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("speaker") || !spec.payload.has("text")) {
        godot::UtilityFunctions::push_error("[ShowDialogueTask] 'speaker' or 'text' is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }
    speaker_ = spec.payload["speaker"];
    text_ = spec.payload["text"];
    return godot::OK;
}

TaskResult ShowDialogueTask::execute() {
    if (runner_) {
        runner_->emit_signal("dialogue_requested", speaker_, text_);
        runner_->set_waiting_for_dialogue(true);
    }
    return TaskResult::Waiting;
}

} // namespace karakuri
