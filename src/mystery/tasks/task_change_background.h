#ifndef MYSTERY_TASK_CHANGE_BACKGROUND_H
#define MYSTERY_TASK_CHANGE_BACKGROUND_H

/**
 * @file task_change_background.h
 * @brief Mystery: 背景の変更を要求する即時完了タスク。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>

namespace mystery {

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

  karakuri::TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;

  void set_background_id(const godot::String &id);
  godot::String get_background_id() const;
};

} // namespace mystery

#endif // MYSTERY_TASK_CHANGE_BACKGROUND_H
