#ifndef KARAKURI_IF_FLAG_TASK_H
#define KARAKURI_IF_FLAG_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/array.hpp>
#include "task_spec.h"

namespace karakuri {

struct IfFlagTaskSpec {
  godot::String key;
  godot::Variant expected_value;
  godot::Array then_branch;
  godot::Array else_branch;
};


class ScenarioRunner;

class IfFlagTask : public TaskBase {
  GDCLASS(IfFlagTask, TaskBase)

  godot::String key_;
  godot::Variant expected_value_ = godot::Variant(true);
  godot::Array then_branch_;
  godot::Array else_branch_;
  
  ScenarioRunner *runner_ = nullptr;

protected:
  static void _bind_methods();

public:
  IfFlagTask() = default;
  ~IfFlagTask() override = default;

  void set_runner(ScenarioRunner *r) override { runner_ = r; }

  TaskResult execute() override;
  godot::Error validate_and_setup(const TaskSpec &spec) override;
  void complete_instantly() override;
};

} // namespace karakuri

#endif // KARAKURI_IF_FLAG_TASK_H
