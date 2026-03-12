#include "set_flag_task.h"
#include "../services/flag_service.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void SetFlagTask::_bind_methods() {}

TaskResult SetFlagTask::execute(double /*delta*/) {
  auto *fs = FlagService::get_singleton();
  if (fs && !key_.is_empty()) {
    fs->set_flag(key_, value_);
  }
  return TaskResult::Success;
}

Error SetFlagTask::validate_and_setup(const Dictionary &spec) {
  if (spec.has("key")) {
    key_ = spec["key"];
    value_ = spec.has("value") ? spec["value"] : Variant(true);
  } else if (spec.has("name")) {
    key_ = spec["name"];
    value_ = spec.has("value") ? spec["value"] : Variant(true);
  } else {
    return ERR_INVALID_DATA;
  }
  return OK;
}

void SetFlagTask::complete_instantly() {
  execute(0.0);
}

} // namespace karakuri
