#ifndef KARAKURI_TASK_BASE_H
#define KARAKURI_TASK_BASE_H

/**
 * @file task_base.h
 * @brief Basic Game Karakuri: 全タスクの基底クラス。
 *
 * ## 役割
 * - on_start() / on_update(delta) / is_finished() の契約を定義する。
 * - complete_instantly() でスキップ時の最終状態を一括適用する。
 *
 * ## 設計ポリシー
 * - RefCounted を継承することで Ref<TaskBase> による自動メモリ管理に対応。
 * - 副作用（フラグ・アイテム更新）は on_start() か complete_instantly() で行い、
 *   on_update() には「段階的な演出」のみを書く。これによりスキップ時の整合性を保証。
 */

#include <godot_cpp/classes/ref_counted.hpp>

namespace karakuri {

class TaskBase : public godot::RefCounted {
  GDCLASS(TaskBase, godot::RefCounted)

protected:
  bool finished_ = false;

  static void _bind_methods();

public:
  TaskBase() = default;
  ~TaskBase() override = default;

  // ------------------------------------------------------------------
  // タスクライフサイクル（サブクラスでオーバーライド）
  // ------------------------------------------------------------------

  /** @brief タスク開始時に一度だけ呼ばれる。副作用はここか complete_instantly() で。 */
  virtual void on_start();

  /** @brief 毎フレーム呼ばれる。演出の進行のみを担う。 */
  virtual void on_update(double delta);

  /** @brief タスクが完了しているかを返す。 */
  virtual bool is_finished() const;

  /**
   * @brief スキップ時に呼ばれる。
   * 演出をスキップし、このタスクがもたらすべき最終状態を即座に適用して終了させる。
   * サブクラスは必ずこれをオーバーライドしてデータ更新を確実に行うこと。
   */
  virtual void complete_instantly();
};

} // namespace karakuri

#endif // KARAKURI_TASK_BASE_H
