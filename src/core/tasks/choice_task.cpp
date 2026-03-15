#include "choice_task.h"
#include "../scenario/scenario_runner.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void ChoiceTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_choices"), &ChoiceTask::get_choices);
}

TaskResult ChoiceTask::execute() {
  if (runner_ == nullptr) {
    return TaskResult::Failed;
  }

  if (!started_) {
    Array choice_texts;
    for(int i = 0; i < choices_.size(); ++i) {
      Dictionary entry = choices_[i];
      Dictionary opt;
      if (entry.has("option")) {
        opt = entry["option"];
      } else {
        opt = entry;
      }

      if (opt.has("text")) {
        choice_texts.append(opt["text"]);
      } else if (opt.has("text_key")) {
        choice_texts.append(opt["text_key"]);
      } else {
        choice_texts.append("...");
      }
    }

    runner_->set_pending_choice_actions(choices_);
    runner_->emit_signal("choices_requested", choice_texts);
    runner_->set_waiting_for_choice(true);
    started_ = true;
  }

  if (runner_->is_waiting_for_choice()) {
    return TaskResult::Waiting;
  }

  return TaskResult::Success;
}

Error ChoiceTask::validate_and_setup(const TaskSpec &spec) {
  ChoiceTaskSpec ts;
  const Dictionary &payload = spec.payload;

  if (payload.has("choices")) {
    ts.choices = payload["choices"];
  } else {
    UtilityFunctions::push_error("ChoiceTask: 'choices' key is missing from spec.");
    return ERR_INVALID_DATA;
  }
  
  choices_ = ts.choices;
  return OK;
}

void ChoiceTask::complete_instantly() {
  if (runner_) {
    runner_->set_waiting_for_choice(false);
  }
}

} // namespace karakuri
