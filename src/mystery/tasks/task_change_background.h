#ifndef MYSTERY_TASK_CHANGE_BACKGROUND_H
#define MYSTERY_TASK_CHANGE_BACKGROUND_H

/**
 * @file task_change_background.h
 * @brief Mystery: 背景の変更を要求する即時完了タスク。
 */

#include "../../core/tasks/task_base.h"
#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>
#include "../../core/tasks/task_spec.h"

namespace mystery {

struct TaskChangeBackgroundSpec {
  godot::String background_id;
};


class TaskChangeBackground : public karakuri::TaskBase {
  GDCLASS(TaskChangeBackground, karakuri::TaskBase)

  godot::String background_id_;

  bool started_ = false;

protected:
  static void _bind_methods();

public:
  TaskChangeBackground() = default;
  ~TaskChangeBackground() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル (ABI v1)
  // ------------------------------------------------------------------

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;

  godot::String get_background_id() const;
};

} // namespace mystery

#endif // MYSTERY_TASK_CHANGE_BACKGROUND_H
