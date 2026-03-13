#ifndef MYSTERY_TASK_ADD_EVIDENCE_H
#define MYSTERY_TASK_ADD_EVIDENCE_H

/**
 * @file task_add_evidence.h
 * @brief Mystery: 証拠品をゲームステートに追加する即時完了タスク。
 *
 * ## 動作
 * - on_start()          : MysteryGameState::add_evidence(evidence_id_) を呼び、
 *                         即座に finished_ = true にする（演出待機なし）。
 * - complete_instantly(): on_start() と同じ処理（スキップ時の整合性を保証）。
 *
 * ## シナリオ YAML / JSON との対応
 * ```yaml
 * - action: add_evidence
 *   value: bloody_knife
 * ```
 * ScenarioRunner の "add_evidence" アクションハンドラがこのタスクを生成して渡す。
 *
 * ## アーキテクチャ境界
 * - src/mystery/ にのみ置く。src/core/ には絶対に入れない。
 * - core 側の TaskBase を継承する（mystery → core は許可された依存方向）。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>

namespace mystery {

class TaskAddEvidence : public karakuri::TaskBase {
  GDCLASS(TaskAddEvidence, karakuri::TaskBase)

  /// 追加する証拠品 ID
  godot::String evidence_id_;

  bool started_ = false;

protected:
  static void _bind_methods();

public:
  TaskAddEvidence() = default;
  ~TaskAddEvidence() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル (ABI v1)
  // ------------------------------------------------------------------

  karakuri::TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;

  // ------------------------------------------------------------------
  // evidence_id プロパティ
  // ------------------------------------------------------------------
  godot::String get_evidence_id() const;
};

} // namespace mystery

#endif // MYSTERY_TASK_ADD_EVIDENCE_H
