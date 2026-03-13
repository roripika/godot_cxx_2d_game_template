#ifndef MYSTERY_LOAD_STATE_TASK_H
#define MYSTERY_LOAD_STATE_TASK_H

/**
 * @file load_state_task.h
 * @brief Mystery: MysteryManager 経由でゲーム状態をロードする即時完了タスク。
 *
 * ## シナリオ YAML との対応
 * ```yaml
 * - action: load
 *   value: mystery
 * ```
 *
 * ## バリデーション
 * - "value" が STRING 型なら demo_id として使用。省略時は "mystery" がデフォルト。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>

namespace mystery {

class LoadStateTask : public karakuri::TaskBase {
  GDCLASS(LoadStateTask, karakuri::TaskBase)

  godot::String demo_id_ = "mystery";
  bool done_ = false;

protected:
  static void _bind_methods();

public:
  LoadStateTask() = default;
  ~LoadStateTask() override = default;

  karakuri::TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;

  godot::String get_demo_id() const { return demo_id_; }
};

} // namespace mystery

#endif // MYSTERY_LOAD_STATE_TASK_H
