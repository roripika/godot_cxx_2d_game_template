#ifndef KARAKURI_CHANGE_ROOT_SCENE_TASK_H
#define KARAKURI_CHANGE_ROOT_SCENE_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class ChangeRootSceneTask : public TaskBase {
  GDCLASS(ChangeRootSceneTask, TaskBase)

  godot::String scene_path_;

protected:
  static void _bind_methods();

public:
  ChangeRootSceneTask() = default;
  ~ChangeRootSceneTask() override = default;

  TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;
};

} // namespace karakuri

#endif // KARAKURI_CHANGE_ROOT_SCENE_TASK_H
