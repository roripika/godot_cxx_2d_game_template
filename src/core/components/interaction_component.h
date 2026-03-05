#ifndef KARAKURI_INTERACTION_COMPONENT_H
#define KARAKURI_INTERACTION_COMPONENT_H

/**
 * @file interaction_component.h
 * @brief Basic Game Karakuri: プレイヤー周囲のインタラクト対象を検知するコンポーネント。
 *
 * ## 役割
 * - detection_radius_ 以内にある IInteractable ノード群の中から
 *   最も近いものを「フォーカス中対象」として保持する。
 * - フォーカスが変わったとき `focus_changed(Node*, bool)` シグナルを発する。
 *   UI はこれを受けて「！」アイコンを表示・非表示する。
 * - InputService 経由で "interact" アクションが押されたとき、
 *   フォーカス中の IInteractable::on_interact() を呼び出す。
 *
 * ## 設計ポリシー (karakuri 層)
 * - mystery 層の具体型（MysteryObject 等）を一切 #include しない。
 * - IInteractable インターフェースのみを介して呼び出す。
 * - "interactable" グループに登録されたノードをスキャンする。
 *   （対象ノード側が _ready() で add_to_group("interactable") を呼ぶ）
 *
 * ## Godot シーンでの使い方
 * 1. MysteryPlayer の子として InteractionComponent ノードを追加する。
 * 2. `detection_radius` をインスペクタで調整する（デフォルト 64px）。
 * 3. `focus_changed(node, is_focused)` シグナルを HUD に接続して
 *    「！」アイコンを表示する。
 */

#include "../interfaces/i_interactable.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node2d.hpp>

namespace karakuri {

class InteractionComponent : public godot::Node {
  GDCLASS(InteractionComponent, godot::Node)

private:
  /// @brief 検知半径 (px)。この距離以内にある IInteractable が候補になる。
  float detection_radius_ = 64.0f;

  /// @brief 現在フォーカス中の IInteractable (nullptr = なし)
  IInteractable *focused_ = nullptr;

  /// @brief focused_ に対応する Godot ノードポインタ（シグナル発信用）
  godot::Node *focused_node_ = nullptr;

  static void _bind_methods();

public:
  InteractionComponent();
  ~InteractionComponent() override = default;

  void _physics_process(double delta) override;

  // ------------------------------------------------------------------
  // Properties
  // ------------------------------------------------------------------

  void set_detection_radius(float radius);
  float get_detection_radius() const;

  // ------------------------------------------------------------------
  // State queries
  // ------------------------------------------------------------------

  /** @brief 現在フォーカス中の対象があるか。 */
  bool has_focus() const;

  /**
   * @brief フォーカス中の Godot ノードを返す（なければ nullptr）。
   * @note GDScript からシグナル引数なしでポーリングしたい場合に使用。
   */
  godot::Node *get_focused_node() const;

  // ------------------------------------------------------------------
  // Signals (定義は _bind_methods で ClassDB::add_signal)
  // ------------------------------------------------------------------
  // focus_changed(node: Node, is_focused: bool)
  //   → node が検知範囲に入った / 出たときに発火。
  //     UI 側でアイコン表示切替に使う。
  //
  // interacted(node: Node)
  //   → on_interact() が実際に呼ばれた直後に発火（エフェクト用）。
};

} // namespace karakuri

#endif // KARAKURI_INTERACTION_COMPONENT_H
