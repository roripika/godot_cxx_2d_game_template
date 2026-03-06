#ifndef MYSTERY_EFFECT_MAP_H
#define MYSTERY_EFFECT_MAP_H

/**
 * @file mystery_effect_map.h
 * @brief Mystery Layer: イベント演出プリセット管理。
 *
 * ## 役割
 * - 「証拠品入手」「逆転」「ダメージ」など特定イベント名に
 *   ActionRunner 演出ステップ / SoundService 設定 を紐づけたプリセットを管理する。
 * - YAML シナリオから `play_effect: "contradict_impact"` と書くだけで
 *   音＋振動＋エフェクトを同時発火できる。
 *
 * ## 組込みプリセット (デフォルト値)
 * | プリセット名       | ActionRunner              | SoundService               |
 * |------------------|---------------------------|----------------------------|
 * | item_collected   | zoom(×1.2, 0.15s)        | se:se_item_get.ogg, vib:0.1s |
 * | contradict_impact| flash+shake               | se:se_objection.ogg, vib:0.4s|
 * | damage           | shake(強)                 | se:se_damage.ogg, vib:0.2s |
 *
 * ## カスタムプリセット登録
 * ```cpp
 * auto *map = mystery::MysteryEffectMap::get_singleton();
 * map->register_preset("my_effect", {
 *   "action_steps": [...],
 *   "se_path": "res://audio/my_se.ogg",
 *   "vibrate_intensity": 1,
 *   "vibrate_duration": 0.2
 * });
 * ```
 */

#include "../core/services/action_runner.h"
#include "../core/services/sound_service.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace mystery {

class MysteryEffectMap : public godot::Node {
  GDCLASS(MysteryEffectMap, godot::Node)

private:
  static MysteryEffectMap *singleton_;

  /// @brief preset_name → { action_steps, se_path, bgm_path, vibrate_intensity, vibrate_duration }
  godot::Dictionary presets_;

  godot::NodePath action_runner_path_;
  godot::NodePath sound_service_path_;

  static void _bind_methods();
  void register_default_presets();

  karakuri::ActionRunner *get_action_runner() const;
  karakuri::SoundService *get_sound_service() const;

public:
  MysteryEffectMap();
  ~MysteryEffectMap() override;

  void _ready() override;

  static MysteryEffectMap *get_singleton();

  // ------------------------------------------------------------------
  // 演出発火 API
  // ------------------------------------------------------------------

  /**
   * @brief プリセット名に対応する演出を発火する。
   * @param preset_name 登録済みプリセット名。未登録の場合は警告のみ。
   */
  void fire(const godot::String &preset_name);

  // ------------------------------------------------------------------
  // プリセット管理
  // ------------------------------------------------------------------

  /**
   * @brief カスタムプリセットを登録・上書きする。
   * @param name   プリセット識別子。
   * @param config { action_steps, se_path, bgm_path,
   *                 vibrate_intensity, vibrate_duration } の Dictionary。
   */
  void register_preset(const godot::String &name,
                       const godot::Dictionary &config);

  godot::Dictionary get_preset(const godot::String &name) const;
  bool has_preset(const godot::String &name) const;

  // ------------------------------------------------------------------
  // プロパティ (エディタ設定)
  // ------------------------------------------------------------------
  void set_action_runner_path(const godot::NodePath &path);
  godot::NodePath get_action_runner_path() const;

  void set_sound_service_path(const godot::NodePath &path);
  godot::NodePath get_sound_service_path() const;
};

} // namespace mystery

#endif // MYSTERY_EFFECT_MAP_H
