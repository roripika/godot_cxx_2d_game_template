#include "change_root_scene_task.h"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void ChangeRootSceneTask::_bind_methods() {}

TaskResult ChangeRootSceneTask::execute(double /*delta*/) {
  // Engine シングルトン経由で SceneTree を取得する
  // 実際には Node の get_tree() が楽だが、Task は Node ではない。
  // ClassDB::instantiate するので、親がない。
  // プロジェクト設定等のシングルトン経由でやるか、
  // とりあえず実装
  return TaskResult::Success; // TODO: 実際の実装 (SceneTree 操作)
}

Error ChangeRootSceneTask::validate_and_setup(const Dictionary &spec) {
  if (spec.has("value")) scene_path_ = spec["value"];
  else if (spec.has("scene_path")) scene_path_ = spec["scene_path"];
  else return ERR_INVALID_DATA;
  return OK;
}

void ChangeRootSceneTask::complete_instantly() {
  execute(0.0);
}

} // namespace karakuri
