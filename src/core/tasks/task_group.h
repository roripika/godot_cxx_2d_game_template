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

  godot::Array tasks_; ///< Ref<TaskBase> の配列

protected:
  static void _bind_methods();

public:
  TaskGroup() = default;
  ~TaskGroup() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル
  // ------------------------------------------------------------------

  /** @brief 全子タスクの on_start() を呼ぶ。 */
  void on_start() override;

  /** @brief 未完了の子タスクを全て on_update() する。 */
  void on_update(double delta) override;

  /** @brief 全子タスクが is_finished() == true のとき true を返す。 */
  bool is_finished() const override;

  /** @brief 全子タスクの complete_instantly() を呼ぶ。 */
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
