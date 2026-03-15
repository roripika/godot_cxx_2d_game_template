#ifndef MYSTERY_PRESENT_EVIDENCE_TASK_H
#define MYSTERY_PRESENT_EVIDENCE_TASK_H

/**
 * @file present_evidence_task.h
 * @brief Mystery: 証拠提示でシナリオを分岐させる非ブロッキングタスク。
 *
 * ## シナリオ YAML との対応
 * ```yaml
 * - action: present_evidence
 *   value:
 *     item_id: bloody_knife
 *     on_correct:
 *       - { action: dialogue, text: "正解です！" }
 *     on_wrong:
 *       - { action: dialogue, text: "違います…" }
 * ```
 *
 * ## バリデーション (Fail-Fast)
 * - "value" が DICTIONARY 型であること。
 * - "value.item_id" が STRING 型で存在すること。
 * - "value.on_correct" または "value.on_wrong" が ARRAY 型で存在すること。
 *
 * ## 動作
 * - ItemService::has_item(item_id_) の結果で on_correct / on_wrong を
 *   ScenarioRunner::inject_steps() で差し込む（Non-blocking）。
 * - runner_ は ScenarioRunner が compile_action() 内で set_runner() で注入する。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include "../../core/tasks/task_spec.h"

namespace karakuri { class ScenarioRunner; }

namespace mystery {

struct PresentEvidenceTaskSpec {
  godot::String item_id;
  godot::Array on_correct;
  godot::Array on_wrong;
};


class PresentEvidenceTask : public karakuri::TaskBase {
  GDCLASS(PresentEvidenceTask, karakuri::TaskBase)

  godot::String item_id_;
  godot::Array on_correct_;
  godot::Array on_wrong_;

  karakuri::ScenarioRunner *runner_ = nullptr;
  bool done_ = false;

protected:
  static void _bind_methods() {}

public:
  PresentEvidenceTask() = default;
  ~PresentEvidenceTask() override = default;

  void set_runner(karakuri::ScenarioRunner *runner) override { runner_ = runner; }

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;

  godot::String get_item_id() const { return item_id_; }
};

} // namespace mystery

#endif // MYSTERY_PRESENT_EVIDENCE_TASK_H
