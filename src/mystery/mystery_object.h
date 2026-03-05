#ifndef MYSTERY_OBJECT_H
#define MYSTERY_OBJECT_H

/**
 * @file mystery_object.h
 * @brief Mystery template: 調査対象オブジェクトの基底クラス。
 *
 * ## 役割
 * IInteractable を実装し、プレイヤーが「調べる」操作をしたときに
 * `interacted(scenario_id)` シグナルを発火させる。
 * GDScript 側でシグナルを受け取り、ScenarioRunner にシナリオを渡す。
 *
 * ## 設計ポリシー (mystery 層)
 * - ScenarioRunner や MysteryManager を直接 #include しない。
 * - シグナル駆動で疎結合を維持する。
 * - `scenario_id` は Godot エディタのインスペクタから設定できる。
 *
 * ## Godot シーンでの使い方
 * 1. MysteryObject ノードを配置し、CollisionShape2D を子として追加する。
 * 2. インスペクタで `scenario_id`（例: "bookshelf_investigation"）を設定する。
 * 3. `interacted(scenario_id)` シグナルを ScenarioRunner に接続する:
 *    ```gdscript
 *    func _on_interacted(id: String) -> void:
 *        scenario_runner.set_scenario_path("res://scenarios/%s.json" % id)
 *        scenario_runner.load_scenario()
 *    ```
 */

#include "../core/interfaces/i_interactable.h"
#include <godot_cpp/classes/static_body2d.hpp>
#include <godot_cpp/variant/string.hpp>

namespace mystery {

/**
 * @brief 調査対象オブジェクト。IInteractable を実装する。
 *
 * StaticBody2D を継承しているため、物理的な衝突形状を持つ。
 * Sprite2D や AnimatedSprite2D は子ノードとしてシーンに追加する。
 */
class MysteryObject : public godot::StaticBody2D,
                      public karakuri::IInteractable {
  GDCLASS(MysteryObject, godot::StaticBody2D)

private:
  /// @brief このオブジェクトに紐付くシナリオ識別子。
  /// 例: "bookshelf_investigation", "desk_clue_01"
  godot::String scenario_id_ = "";

  /// @brief 調査アイコンに表示するカスタムラベル（省略可）。
  /// 空の場合 InteractionComponent がデフォルトアイコンを使う。
  godot::String interaction_label_ = "";

  static void _bind_methods();

public:
  MysteryObject();
  ~MysteryObject() override = default;

  void _ready() override;

  // ------------------------------------------------------------------
  // IInteractable 実装
  // ------------------------------------------------------------------

  /**
   * @brief InteractionComponent から呼ばれる。
   * `interacted` シグナルを発火し、シナリオ起動をシーン側に委譲する。
   */
  void on_interact() override;

  /**
   * @brief 調査アイコンに表示するラベルを返す。
   */
  godot::String get_interaction_label() const override;

  // ------------------------------------------------------------------
  // Properties
  // ------------------------------------------------------------------

  void set_scenario_id(const godot::String &id);
  godot::String get_scenario_id() const;

  void set_interaction_label_prop(const godot::String &label);
  godot::String get_interaction_label_prop() const;

  // ------------------------------------------------------------------
  // Signals (定義は _bind_methods で ClassDB::add_signal)
  // ------------------------------------------------------------------
  // interacted(scenario_id: String)
  //   → on_interact() が呼ばれたとき発火。
  //     シーン側の GDScript でシナリオ開始処理に接続する。
};

} // namespace mystery

#endif // MYSTERY_OBJECT_H
