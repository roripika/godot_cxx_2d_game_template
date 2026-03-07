#ifndef MYSTERY_PLAY_MYSTERY_SOUND_TASK_H
#define MYSTERY_PLAY_MYSTERY_SOUND_TASK_H

/**
 * @file play_mystery_sound_task.h
 * @brief Mystery: MysteryEffectMap プリセット or 直接 SE パスを即時再生するタスク。
 *
 * ## 動作
 * - on_start() で MysteryEffectMap::fire(preset_name) もしくは
 *   SoundService::play_se(se_path) を呼び、即座に finished_ = true にする。
 * - fire-and-forget 型のため is_finished() は最初の on_update() 前から true を返す。
 * - complete_instantly() も同様に即時再生して完了する。
 *
 * ## 使い方（GDScript）
 * ```gdscript
 * # プリセット使用
 * var task = PlayMysterySoundTask.new()
 * task.set_preset_name("contradict_impact")
 * task.set_effect_map_path(NodePath("../MysteryEffectMap"))
 *
 * # 直接 SE パス使用
 * var task2 = PlayMysterySoundTask.new()
 * task2.set_se_path("res://assets/sounds/collect.ogg")
 * ```
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>

namespace mystery {

class PlayMysterySoundTask : public karakuri::TaskBase {
  GDCLASS(PlayMysterySoundTask, karakuri::TaskBase)

  godot::NodePath effect_map_path_;
  godot::String preset_name_; ///< セットすると MysteryEffectMap::fire() を使う
  godot::String se_path_;     ///< セットすると SoundService::play_se() を使う（preset が空の場合）

protected:
  static void _bind_methods();
  void play_sound();

public:
  PlayMysterySoundTask() = default;
  ~PlayMysterySoundTask() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル
  // ------------------------------------------------------------------
  void on_start() override;
  void complete_instantly() override;

  // ------------------------------------------------------------------
  // プロパティ
  // ------------------------------------------------------------------
  void set_effect_map_path(const godot::NodePath &path);
  godot::NodePath get_effect_map_path() const;

  void set_preset_name(const godot::String &name);
  godot::String get_preset_name() const;

  void set_se_path(const godot::String &path);
  godot::String get_se_path() const;
};

} // namespace mystery

#endif // MYSTERY_PLAY_MYSTERY_SOUND_TASK_H
