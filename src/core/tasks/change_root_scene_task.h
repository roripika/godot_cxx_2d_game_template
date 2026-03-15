#ifndef KARAKURI_CHANGE_ROOT_SCENE_TASK_H
#define KARAKURI_CHANGE_ROOT_SCENE_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>
#include "task_spec.h"

namespace karakuri {

struct ChangeRootSceneTaskSpec {
  godot::String scene_path;
  godot::Dictionary params;
};

class ScenarioRunner;

class ChangeRootSceneTask : public TaskBase {
  GDCLASS(ChangeRootSceneTask, TaskBase)

  godot::String scene_path_;
  godot::Dictionary params_;
  bool transition_requested_ = false;

  ScenarioRunner *runner_ = nullptr;

protected:
  static void _bind_methods();

public:
  ChangeRootSceneTask() = default;
  ~ChangeRootSceneTask() override = default;

  void set_runner(ScenarioRunner *runner) override { runner_ = runner; }

  TaskResult execute() override;
  godot::Error validate_and_setup(const TaskSpec &spec) override;
  void complete_instantly() override;
};

} // namespace karakuri

#endif // KARAKURI_CHANGE_ROOT_SCENE_TASK_H
