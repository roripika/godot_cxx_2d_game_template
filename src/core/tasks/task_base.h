#ifndef KARAKURI_TASK_BASE_H
#define KARAKURI_TASK_BASE_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/core/error_macros.hpp>

namespace karakuri {

enum class TaskResult {
  Success,  // 完了。次へ。
  Waiting,  // 外部入力待ち。次フレームで再開。
  Failed,   // 致命的エラー。実行停止。
  Yielded   // 1フレーム待機（演出継続中）。
};

class ScenarioRunner;

class TaskBase : public godot::RefCounted {
  GDCLASS(TaskBase, godot::RefCounted)

protected:
  static void _bind_methods();

public:
  TaskBase() = default;
  virtual ~TaskBase() override = default;

  /**
   * @brief タスクを実行する。毎フレーム呼ばれる。
   * @param delta フレーム間の差分時間
   * @return TaskResult 実行結果
   */
  virtual TaskResult execute(double delta) = 0;

  /**
   * @brief シナリオデータ（Dictionary）を受け取り、初期化・検証を行う。
   * ロード時に一度だけ呼ばれる。必須パラメータが欠けている場合は Error を返す。
   * @param spec タスクの定義データ
   * @return Error OK またはエラーコード
   */
  virtual godot::Error validate_and_setup(const godot::Dictionary &spec) = 0;

  /**
   * @brief スキップ時に呼ばれる。
   * 演出をスキップし、このタスクがもたらすべき最終状態を即座に適用して終了させる。
   */
  virtual void complete_instantly() = 0;

  /**
   * @brief Runner コンテキストの注入 (Option)
   */
  virtual void set_runner(ScenarioRunner *runner) {}
};

} // namespace karakuri

#endif // KARAKURI_TASK_BASE_H
