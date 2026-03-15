#ifndef MYSTERY_RESET_GAME_TASK_H
#define MYSTERY_RESET_GAME_TASK_H

#include "../../core/tasks/task_base.h"
#include "../../core/tasks/task_spec.h"

namespace mystery {

struct ResetGameTaskSpec {};


class ResetGameTask : public karakuri::TaskBase {
  GDCLASS(ResetGameTask, karakuri::TaskBase)

protected:
  static void _bind_methods() {}

public:
  ResetGameTask() = default;
  ~ResetGameTask() override = default;

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;
};

} // namespace mystery

#endif // MYSTERY_RESET_GAME_TASK_H
