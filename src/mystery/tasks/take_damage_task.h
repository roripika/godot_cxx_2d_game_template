#ifndef MYSTERY_TAKE_DAMAGE_TASK_H
#define MYSTERY_TAKE_DAMAGE_TASK_H

#include "../../core/tasks/task_base.h"
#include "../../core/tasks/task_spec.h"

namespace mystery {

struct TakeDamageTaskSpec {
  int amount = 1;
};


class TakeDamageTask : public karakuri::TaskBase {
  GDCLASS(TakeDamageTask, karakuri::TaskBase)

  int amount_ = 1;

protected:
  static void _bind_methods() {}

public:
  TakeDamageTask() = default;
  ~TakeDamageTask() override = default;

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;
};

} // namespace mystery

#endif // MYSTERY_TAKE_DAMAGE_TASK_H
