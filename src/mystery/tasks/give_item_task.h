#ifndef MYSTERY_GIVE_ITEM_TASK_H
#define MYSTERY_GIVE_ITEM_TASK_H

/**
 * @file give_item_task.h
 * @brief Mystery: ItemService 経由でインベントリにアイテムを追加する即時完了タスク。
 *
 * ## シナリオ YAML との対応
 * ```yaml
 * - action: give_item
 *   value: key_card
 * ```
 *
 * ## バリデーション (Fail-Fast)
 * - "value" または "item_id" キー（STRING 型）が必須。なければ ERR_INVALID_DATA。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>

namespace mystery {

class GiveItemTask : public karakuri::TaskBase {
  GDCLASS(GiveItemTask, karakuri::TaskBase)

  godot::String item_id_;
  bool done_ = false;

protected:
  static void _bind_methods();

public:
  GiveItemTask() = default;
  ~GiveItemTask() override = default;

  karakuri::TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;

  godot::String get_item_id() const { return item_id_; }
};

} // namespace mystery

#endif // MYSTERY_GIVE_ITEM_TASK_H
