#ifndef KARAKURI_CHOICE_TASK_H
#define KARAKURI_CHOICE_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/array.hpp>

namespace karakuri {

class ScenarioRunner;

class ChoiceTask : public TaskBase {
  GDCLASS(ChoiceTask, TaskBase)

  godot::Array choices_;
  bool started_ = false;
  
  ScenarioRunner *runner_ = nullptr;

protected:
  static void _bind_methods();

public:
  ChoiceTask() = default;
  ~ChoiceTask() override = default;

  void set_runner(ScenarioRunner *runner) { runner_ = runner; }

  TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;

  void set_choices(const godot::Array &choices) { choices_ = choices; }
  godot::Array get_choices() const { return choices_; }
};

} // namespace karakuri

#endif // KARAKURI_CHOICE_TASK_H
