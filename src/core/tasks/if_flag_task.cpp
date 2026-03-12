#include "if_flag_task.h"
#include "../scenario/scenario_runner.h"
#include "../services/flag_service.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void IfFlagTask::_bind_methods() {}

TaskResult IfFlagTask::execute(double /*delta*/) {
  if (runner_ == nullptr) return TaskResult::Failed;

  auto *fs = FlagService::get_singleton();
  bool result = false;
  if (fs) {
    Variant current = fs->get_flag(key_);
    result = (current == expected_value_);
  }

  const Array &branch = result ? then_branch_ : else_branch_;
  if (!branch.is_empty()) {
    runner_->inject_steps(branch);
  }

  return TaskResult::Success;
}

Error IfFlagTask::validate_and_setup(const Dictionary &spec) {
  if (spec.has("key")) {
    key_ = spec["key"];
    expected_value_ = spec.has("value") ? spec["value"] : Variant(true);
    then_branch_ = spec.has("then") ? Array(spec["then"]) : Array();
    else_branch_ = spec.has("else") ? Array(spec["else"]) : Array();
  } else {
    return ERR_INVALID_DATA;
  }
  return OK;
}

void IfFlagTask::complete_instantly() {
  execute(0.0);
}

} // namespace karakuri
