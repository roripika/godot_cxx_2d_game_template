#ifndef KARAKURI_GOTO_TASK_H
#define KARAKURI_GOTO_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>
#include "task_spec.h"

namespace karakuri {

struct GotoTaskSpec {
  godot::String scene_id;
};


class ScenarioRunner;

class GotoTask : public TaskBase {
  GDCLASS(GotoTask, TaskBase)

  godot::String scene_id_;
  ScenarioRunner *runner_ = nullptr;

protected:
  static void _bind_methods();

public:
  GotoTask() = default;
  ~GotoTask() override = default;

  void set_runner(ScenarioRunner *runner) override { runner_ = runner; }

  TaskResult execute() override;
  godot::Error validate_and_setup(const TaskSpec &spec) override;
  void complete_instantly() override;

  godot::String get_scene_id() const { return scene_id_; }
};

} // namespace karakuri

#endif // KARAKURI_GOTO_TASK_H
