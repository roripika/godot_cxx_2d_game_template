#include "goto_task.h"
#include "../scenario/scenario_runner.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void GotoTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_scene_id"), &GotoTask::get_scene_id);
}

TaskResult GotoTask::execute(double /*delta*/) {
  if (runner_) {
    runner_->load_scene_by_id(scene_id_);
  }
  return TaskResult::Success;
}

Error GotoTask::validate_and_setup(const Dictionary &spec) {
  if (spec.has("scene_id")) {
    scene_id_ = spec["scene_id"];
  } else if (spec.has("value")) {
    scene_id_ = spec["value"];
  } else {
    UtilityFunctions::push_error("GotoTask: 'scene_id' (or 'value') key is missing from spec.");
    return ERR_INVALID_DATA;
  }
  return OK;
}

void GotoTask::complete_instantly() {
  execute(0.0);
}

} // namespace karakuri
