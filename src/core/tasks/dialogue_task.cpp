#include "dialogue_task.h"
#include "../scenario/scenario_runner.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void DialogueTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_speaker"), &DialogueTask::get_speaker);
  ClassDB::bind_method(D_METHOD("get_text"), &DialogueTask::get_text);
}

TaskResult DialogueTask::execute(double /*delta*/) {
  if (runner_ == nullptr) {
    return TaskResult::Failed;
  }

  if (!started_) {
    runner_->emit_signal("dialogue_requested", speaker_, text_);
    runner_->set_waiting_for_dialogue(true);
    started_ = true;
  }

  if (runner_->is_waiting_for_dialogue()) {
    return TaskResult::Waiting;
  }

  return TaskResult::Success;
}

Error DialogueTask::validate_and_setup(const Dictionary &spec) {
  // specs["speaker"] または specs["text"] を探す
  // 旧形式 { "dialogue": { "speaker": "...", "text": "..." } } はマッピング時に展開される想定
  if (spec.has("speaker")) {
    speaker_ = spec["speaker"];
  } else if (spec.has("speaker_key")) {
    speaker_ = spec["speaker_key"];
  }

  if (spec.has("text")) {
    text_ = spec["text"];
  } else if (spec.has("text_key")) {
    text_ = spec["text_key"];
  } else {
    UtilityFunctions::push_error("DialogueTask: 'text' (or 'text_key') key is missing or not a String in spec.");
    return ERR_INVALID_DATA;
  }
  
  return OK;
}

void DialogueTask::complete_instantly() {
  if (runner_) {
    runner_->set_waiting_for_dialogue(false);
  }
}

} // namespace karakuri
