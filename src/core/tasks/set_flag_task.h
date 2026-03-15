#ifndef KARAKURI_SET_FLAG_TASK_H
#define KARAKURI_SET_FLAG_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>
#include "task_spec.h"

namespace karakuri {

struct SetFlagTaskSpec {
  godot::String key;
  godot::Variant value;
};


class SetFlagTask : public TaskBase {
  GDCLASS(SetFlagTask, TaskBase)

  godot::String key_;
  godot::Variant value_;

protected:
  static void _bind_methods();

public:
  SetFlagTask() = default;
  ~SetFlagTask() override = default;

  TaskResult execute() override;
  godot::Error validate_and_setup(const TaskSpec &spec) override;
  void complete_instantly() override;
};

} // namespace karakuri

#endif // KARAKURI_SET_FLAG_TASK_H
