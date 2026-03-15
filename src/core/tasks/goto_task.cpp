#include "goto_task.h"
#include "../scenario/scenario_runner.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void GotoTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_scene_id"), &GotoTask::get_scene_id);
}

TaskResult GotoTask::execute() {
  if (runner_) {
    godot::Dictionary empty_params;
    runner_->request_transition(scene_id_, empty_params);
  }
  return TaskResult::Waiting;
}

Error GotoTask::validate_and_setup(const TaskSpec &spec) {
  GotoTaskSpec ts;
  const Dictionary &payload = spec.payload;

  if (payload.has("scene_id")) {
    ts.scene_id = payload["scene_id"];
  } else if (payload.has("value")) {
    ts.scene_id = payload["value"];
  } else {
    UtilityFunctions::push_error("GotoTask: 'scene_id' (or 'value') key is missing from spec.");
    return ERR_INVALID_DATA;
  }
  
  scene_id_ = ts.scene_id;
  return OK;
}

void GotoTask::complete_instantly() {
  execute();
}

} // namespace karakuri
