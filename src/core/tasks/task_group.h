#ifndef KARAKURI_TASK_GROUP_H
#define KARAKURI_TASK_GROUP_H

/**
 * @file task_group.h
 * @brief Basic Game Karakuri: 複数タスクを並列実行するコンテナタスク。
 *
 * ## 役割
 * TaskGroup は複数の TaskBase を保持し、全てのタスクが is_finished() == true に
 * なるまで on_update() を呼び続ける「AND ゲート」として機能する。
 *
 * ## 使い方（GDScript）
 * ```gdscript
 * var group = TaskGroup.new()
 * group.add_task(zoom_task)
 * group.add_task(sound_task)
 * sequence_player.add_task(group)
 * ```
 */

#include "task_base.h"
#include <godot_cpp/variant/array.hpp>

namespace karakuri {

class TaskGroup : public TaskBase {
  GDCLASS(TaskGroup, TaskBase)

  godot::Array tasks_;           ///< Ref<TaskBase> の配列
  godot::Array completed_flags_; ///< tasks_ と同サイズの bool 配列
  bool finished_ = false;

protected:
  static void _bind_methods();

public:
  TaskGroup() = default;
  ~TaskGroup() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル (ABI v1)
  // ------------------------------------------------------------------

  TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;

  // ------------------------------------------------------------------
  // タスク管理
  // ------------------------------------------------------------------
  void add_task(const godot::Ref<TaskBase> &task);
  void clear_tasks();
  int get_task_count() const;
};

} // namespace karakuri

#endif // KARAKURI_TASK_GROUP_H
