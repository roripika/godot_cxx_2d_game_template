#ifndef MYSTERY_TASK_SHOW_PORTRAIT_H
#define MYSTERY_TASK_SHOW_PORTRAIT_H

/**
 * @file task_show_portrait.h
 * @brief Mystery: キャラクター立ち絵の表示を要求する即時完了タスク。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>

namespace mystery {

class TaskShowPortrait : public karakuri::TaskBase {
  GDCLASS(TaskShowPortrait, karakuri::TaskBase)

  godot::String character_id_;
  godot::String emotion_ = "default";

protected:
  static void _bind_methods();

public:
  TaskShowPortrait() = default;
  ~TaskShowPortrait() override = default;

  void on_start() override;
  void complete_instantly() override;

  void set_character_id(const godot::String &id);
  godot::String get_character_id() const;

  void set_emotion(const godot::String &emotion);
  godot::String get_emotion() const;
};

} // namespace mystery

#endif // MYSTERY_TASK_SHOW_PORTRAIT_H
