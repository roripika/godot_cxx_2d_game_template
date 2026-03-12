#ifndef MYSTERY_RESET_GAME_TASK_H
#define MYSTERY_RESET_GAME_TASK_H

#include "../../core/tasks/task_base.h"

namespace mystery {

class ResetGameTask : public karakuri::TaskBase {
  GDCLASS(ResetGameTask, karakuri::TaskBase)

protected:
  static void _bind_methods() {}

public:
  ResetGameTask() = default;
  ~ResetGameTask() override = default;

  karakuri::TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;
};

} // namespace mystery

#endif // MYSTERY_RESET_GAME_TASK_H
