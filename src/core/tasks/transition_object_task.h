#ifndef KARAKURI_TRANSITION_OBJECT_TASK_H
#define KARAKURI_TRANSITION_OBJECT_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace karakuri {

class ScenarioRunner;

class TransitionObjectTask : public TaskBase {
  GDCLASS(TransitionObjectTask, TaskBase)

  godot::Dictionary params_;
  ScenarioRunner *runner_ = nullptr;
  bool finished_ = false;

protected:
  static void _bind_methods();

public:
  TransitionObjectTask() = default;
  ~TransitionObjectTask() override = default;

  void set_runner(ScenarioRunner *runner) { runner_ = runner; }

  TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;
};

} // namespace karakuri

#endif // KARAKURI_TRANSITION_OBJECT_TASK_H
