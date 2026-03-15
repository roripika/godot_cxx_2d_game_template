#ifndef MYSTERY_GIVE_EVIDENCE_TASK_H
#define MYSTERY_GIVE_EVIDENCE_TASK_H

/**
 * @file give_evidence_task.h
 * @brief Mystery: 証拠品を EvidenceManager に追加する即時完了タスク。
 *
 * ## シナリオ YAML との対応
 * ```yaml
 * - action: give_evidence
 *   value: bloody_knife
 * ```
 * または
 * ```yaml
 * - action: give_evidence
 *   evidence_id: bloody_knife
 * ```
 *
 * ## バリデーション (Fail-Fast)
 * - "value" または "evidence_id" キーが必須。なければ ERR_INVALID_DATA。
 *
 * ## アーキテクチャ境界
 * - src/mystery/ 内のみ。src/core/ を include しない（EvidenceManager は mystery層）。
 * - TaskBase を継承（mystery → core は許可された依存方向）。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>
#include "../../core/tasks/task_spec.h"

namespace mystery {

struct GiveEvidenceTaskSpec {
  godot::String evidence_id;
};


class GiveEvidenceTask : public karakuri::TaskBase {
  GDCLASS(GiveEvidenceTask, karakuri::TaskBase)

  godot::String evidence_id_;
  bool done_ = false;

protected:
  static void _bind_methods();

public:
  GiveEvidenceTask() = default;
  ~GiveEvidenceTask() override = default;

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;

  godot::String get_evidence_id() const { return evidence_id_; }
};

} // namespace mystery

#endif // MYSTERY_GIVE_EVIDENCE_TASK_H
