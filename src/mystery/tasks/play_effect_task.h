#ifndef MYSTERY_PLAY_EFFECT_TASK_H
#define MYSTERY_PLAY_EFFECT_TASK_H

/**
 * @file play_effect_task.h
 * @brief Mystery: MysteryEffectMap のプリセットを発火する即時完了タスク。
 *
 * ## シナリオ YAML との対応
 * ```yaml
 * - action: play_effect
 *   value: contradict_impact
 * ```
 *
 * ## バリデーション (Fail-Fast)
 * - "value" または "preset" キー（STRING 型）が必須。なければ ERR_INVALID_DATA。
 *
 * ## アーキテクチャ境界
 * - src/mystery/ 内のみ。MysteryEffectMap は mystery 層。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/string.hpp>
#include "../../core/tasks/task_spec.h"

namespace mystery {

struct PlayEffectTaskSpec {
  godot::String preset;
};


class PlayEffectTask : public karakuri::TaskBase {
  GDCLASS(PlayEffectTask, karakuri::TaskBase)

  godot::String preset_;
  bool done_ = false;

protected:
  static void _bind_methods();

public:
  PlayEffectTask() = default;
  ~PlayEffectTask() override = default;

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;

  godot::String get_preset() const { return preset_; }
};

} // namespace mystery

#endif // MYSTERY_PLAY_EFFECT_TASK_H
