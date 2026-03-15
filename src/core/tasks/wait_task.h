#ifndef KARAKURI_WAIT_TASK_H
#define KARAKURI_WAIT_TASK_H

/**
 * @file wait_task.h
 * @brief Basic Game Karakuri: 秒数またはシグナル待機タスク。
 *
 * ## 使い方
 * ```yaml
 * # 秒数待機
 * - action: wait
 *   duration: 2.0
 *
 * # シグナル待機（duration を省略または 0 にすると mark_signal_received() 待ちになる）
 * - action: wait
 * ```
 *
 * ## スキップ
 * complete_instantly() を呼ぶと target_time_ = now() にセットされ、
 * is_finished() が即座に true を返すようになる。シグナル待機の場合も同様。
 */

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>
#include "task_spec.h"

namespace karakuri {

enum class WaitMode {
  Timer,
  Signal
};

struct WaitTaskSpec {
  double duration;
};


class WaitTask : public TaskBase {
  GDCLASS(WaitTask, TaskBase)

  WaitMode mode_ = WaitMode::Timer;
  double duration_ = 0.0;
  double target_time_ = 0.0;
  bool signal_received_ = false;

  bool started_ = false;

protected:
  static void _bind_methods();

public:
  WaitTask() = default;
  ~WaitTask() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル (ABI v1)
  // ------------------------------------------------------------------
  TaskResult execute() override;
  godot::Error validate_and_setup(const TaskSpec &spec) override;
  void complete_instantly() override;

  // ------------------------------------------------------------------
  // シグナル待機サポート
  // ------------------------------------------------------------------

  /**
   * @brief 外部シグナルからこのメソッドを呼ぶとタスクが完了する。
   *
   * ## 使い方（GDScript）
   * ```gdscript
   * var task = WaitTask.new()
   * # duration を設定しないとシグナル待機モードになる
   * some_node.connect("animation_finished", task.mark_signal_received)
   * sequence_player.add_task(task)
   * ```
   */
  void mark_signal_received();

  // ------------------------------------------------------------------
  // プロパティアクセサ
  // ------------------------------------------------------------------
  double get_duration() const;
};

} // namespace karakuri

#endif // KARAKURI_WAIT_TASK_H
