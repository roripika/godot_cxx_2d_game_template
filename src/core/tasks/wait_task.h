#ifndef KARAKURI_WAIT_TASK_H
#define KARAKURI_WAIT_TASK_H

/**
 * @file wait_task.h
 * @brief Basic Game Karakuri: 秒数またはシグナル待機タスク。
 *
 * ## 使い方
 * ```cpp
 * // 秒数待機
 * Ref<WaitTask> t = memnew(WaitTask);
 * t->set_duration(2.0f);
 *
 * // シグナル待機（OBJECT_PATH:signal_name 形式）
 * Ref<WaitTask> t2 = memnew(WaitTask);
 * t2->set_signal_source("res://ui/dialogue_box.tscn:dialogue_finished");
 * ```
 *
 * ## スキップ
 * complete_instantly() を呼ぶと elapsed_ = duration_ にセットされ、
 * is_finished() が即座に true を返すようになる。シグナル待機の場合も同様。
 */

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class WaitTask : public TaskBase {
  GDCLASS(WaitTask, TaskBase)

  double duration_ = 0.0;
  double elapsed_ = 0.0;
  bool signal_received_ = false;

protected:
  static void _bind_methods();

public:
  WaitTask() = default;
  ~WaitTask() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル
  // ------------------------------------------------------------------
  void on_start() override;
  void on_update(double delta) override;
  bool is_finished() const override;
  void complete_instantly() override;

  // ------------------------------------------------------------------
  // プロパティアクセサ
  // ------------------------------------------------------------------
  void set_duration(double seconds);
  double get_duration() const;
};

} // namespace karakuri

#endif // KARAKURI_WAIT_TASK_H
