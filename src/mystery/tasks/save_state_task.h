#ifndef MYSTERY_SAVE_STATE_TASK_H
#define MYSTERY_SAVE_STATE_TASK_H

/**
 * @file save_state_task.h
 * @brief Mystery: MysteryManager 経由でゲーム状態をセーブする即時完了タスク。
 *
 * ## シナリオ YAML との対応
 * ```yaml
 * - action: save
 *   value: mystery
 * ```
 *
 * ## バリデーション
 * - "value" が STRING 型なら demo_id として使用。省略時は "mystery" がデフォルト。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>
#include "../../core/tasks/task_spec.h"

namespace mystery {

struct SaveStateTaskSpec {
  godot::String demo_id = "mystery";
};


class SaveStateTask : public karakuri::TaskBase {
  GDCLASS(SaveStateTask, karakuri::TaskBase)

  godot::String demo_id_ = "mystery";
  bool done_ = false;

protected:
  static void _bind_methods();

public:
  SaveStateTask() = default;
  ~SaveStateTask() override = default;

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;

  godot::String get_demo_id() const { return demo_id_; }
};

} // namespace mystery

#endif // MYSTERY_SAVE_STATE_TASK_H
