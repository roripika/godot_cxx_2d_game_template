#ifndef MYSTERY_GAME_STATE_H
#define MYSTERY_GAME_STATE_H

/**
 * @file mystery_game_state.h
 * @brief Mystery テンプレート専用ゲームステート。
 *
 * ## 責務（Mystery 境界内）
 * - HP 管理（デフォルト 3 回のミス許容）
 * - health_changed シグナルの発火
 * - reset_game() のオーバーライド（HP を 3 にリセット）
 *
 * ## 継承元
 * karakuri::KarakuriGameState — リセットフック管理を継承する。
 *
 * ## Godot クラス階層
 * Node
 *   ↳ KarakuriGameState  (reset_hook のみ。他ゲームでも再利用可)
 *       ↳ MysteryGameState (HP・証拠フラグ等、Mystery 固有の状態)
 *
 * ## シングルトンについて
 * MysteryGameState のコンストラクタは KarakuriGameState::singleton_ も自身のポインタで
 * 更新するため、`KarakuriGameState::get_singleton()` 経由でも MysteryGameState インスタンスを
 * 取得できる（is-a 関係を活用）。
 */

#include <godot_cpp/variant/typed_array.hpp>

#include "core/karakuri_game_state.h"

namespace mystery {

class MysteryGameState : public karakuri::KarakuriGameState {
  GDCLASS(MysteryGameState, karakuri::KarakuriGameState)

  static MysteryGameState *singleton_;

  /// HP（ミス許容回数）。デフォルト 3。
  int health_ = 3;

  /// 収集済み証拠品 ID のリスト
  godot::TypedArray<godot::String> collected_evidences_;

protected:
  static void _bind_methods();

  /** @brief ノード通知ハンドラ。NOTIFICATION_READY で _ready() を呼び出す。 */
  void _notification(int p_what);

public:
  MysteryGameState();
  ~MysteryGameState() override;

  /** @brief 唯一の MysteryGameState インスタンスを返す。 */
  static MysteryGameState *get_singleton();

  /** @brief Autoload に追加された直後、Mystery 固有のアクションを ActionRegistry に登録する。 */
  void _ready() override;

  // ------------------------------------------------------------------
  // HP 管理
  // ------------------------------------------------------------------

  /** @brief HP を直接設定し、health_changed を発火する。 */
  void set_health(int hp);

  /** @brief 現在の HP を返す。 */
  int get_health() const;

  /** @brief 1 ダメージを受け、health_changed を発火する。 */
  void take_damage();

  /** @brief amount だけ回復し、health_changed を発火する。 */
  void heal(int amount);

  /** @brief HP を初期値 3 に戻し、health_changed を発火する。 */
  void reset_health();

  // ------------------------------------------------------------------
  // 証拠品管理
  // ------------------------------------------------------------------

  /**
   * @brief 証拠品 ID をリストに追加し、evidence_added を発火する。
   * 既に持っている場合は何もしない。
   */
  void add_evidence(const godot::String &evidence_id);

  /**
   * @brief 指定の証拠品 ID をすでに持っているか判定する。
   */
  bool has_evidence(const godot::String &evidence_id) const;

  /** @brief 収集済み証拠品リストを返す。 */
  godot::TypedArray<godot::String> get_collected_evidences() const;

  // ------------------------------------------------------------------
  // リセットのオーバーライド
  // ------------------------------------------------------------------

  /**
   * @brief ゲームをリセットする。
   * 基盤の reset_hook_ を呼び出した後、Mystery 固有の HP を 3 に戻す。
   */
  void reset_game() override;
};

} // namespace mystery

#endif // MYSTERY_GAME_STATE_H
