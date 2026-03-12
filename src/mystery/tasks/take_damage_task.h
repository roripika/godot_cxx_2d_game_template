#ifndef MYSTERY_TAKE_DAMAGE_TASK_H
#define MYSTERY_TAKE_DAMAGE_TASK_H

#include "../../core/tasks/task_base.h"

namespace mystery {

class TakeDamageTask : public karakuri::TaskBase {
  GDCLASS(TakeDamageTask, karakuri::TaskBase)

  int amount_ = 1;

protected:
  static void _bind_methods() {}

public:
  TakeDamageTask() = default;
  ~TakeDamageTask() override = default;

  karakuri::TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;
};

} // namespace mystery

#endif // MYSTERY_TAKE_DAMAGE_TASK_H
