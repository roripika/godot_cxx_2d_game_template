#ifndef KARAKURI_IF_HAS_ITEMS_TASK_H
#define KARAKURI_IF_HAS_ITEMS_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class ScenarioRunner;

class IfHasItemsTask : public TaskBase {
  GDCLASS(IfHasItemsTask, TaskBase)

  godot::Array required_items_;
  godot::Array then_branch_;
  godot::Array else_branch_;
  
  ScenarioRunner *runner_ = nullptr;

protected:
  static void _bind_methods();

public:
  IfHasItemsTask() = default;
  ~IfHasItemsTask() override = default;

  void set_runner(ScenarioRunner *runner) override { runner_ = runner; }

  TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;
};

} // namespace karakuri

#endif // KARAKURI_IF_HAS_ITEMS_TASK_H
