#include "change_root_scene_task.h"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "../scenario/scenario_runner.h"

using namespace godot;

namespace karakuri {

void ChangeRootSceneTask::_bind_methods() {}

TaskResult ChangeRootSceneTask::execute() {
  if (!transition_requested_) {
    if (runner_) {
      runner_->request_transition(scene_path_, params_);
    }
    transition_requested_ = true;
    return TaskResult::Waiting;
  }

  if (runner_ && !runner_->is_waiting_for_transition()) {
    return TaskResult::Success;
  }

  return TaskResult::Waiting;
}

godot::Error ChangeRootSceneTask::validate_and_setup(const TaskSpec &spec) {
  ChangeRootSceneTaskSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("value")) ts.scene_path = payload["value"];
  else if (payload.has("scene_path")) ts.scene_path = payload["scene_path"];
  else return godot::ERR_INVALID_DATA;

  ts.params = payload;

  scene_path_ = ts.scene_path;
  params_ = ts.params;
  return godot::OK;
}

void ChangeRootSceneTask::complete_instantly() {
  execute();
}

} // namespace karakuri
