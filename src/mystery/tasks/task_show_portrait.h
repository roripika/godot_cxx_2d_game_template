#ifndef MYSTERY_TASK_SHOW_PORTRAIT_H
#define MYSTERY_TASK_SHOW_PORTRAIT_H

/**
 * @file task_show_portrait.h
 * @brief Mystery: キャラクター立ち絵の表示を要求する即時完了タスク。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>
#include "../../core/tasks/task_spec.h"

namespace mystery {

struct TaskShowPortraitSpec {
  godot::String character_id;
  godot::String emotion;
};


class TaskShowPortrait : public karakuri::TaskBase {
  GDCLASS(TaskShowPortrait, karakuri::TaskBase)

  godot::String character_id_;
  godot::String emotion_ = "default";

  bool started_ = false;

protected:
  static void _bind_methods();

public:
  TaskShowPortrait() = default;
  ~TaskShowPortrait() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル (ABI v1)
  // ------------------------------------------------------------------

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;

  godot::String get_character_id() const;

  godot::String get_emotion() const;
};

} // namespace mystery

#endif // MYSTERY_TASK_SHOW_PORTRAIT_H
