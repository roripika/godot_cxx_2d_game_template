#ifndef INVADER_MANAGER_H
#define INVADER_MANAGER_H

/**
 * @file invader_manager.h
 * @brief インベーダーゲーム: ゲームサイクル管理クラス。
 *
 * ## 責務
 * - スコア管理（`add_score` → `score_changed` シグナル）
 * - 敵全滅検出（グループ "invader_enemies" のノード数が 0 → `game_clear`）
 * - 敵が下限 Y 座標を超えた検出 → `game_over`
 * - `notify_player_hit()` — GDScript から呼ばれてプレイヤー被弾を通知 →
 * `game_over`
 *
 * ## UI 結線ルール
 * C++ 内にノード操作を書かない。
 * `score_changed` / `game_over` / `game_clear` を GDScript View 層が受け取り UI
 * を更新する。
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>

namespace invaders {

class InvaderManager : public godot::Node {
  GDCLASS(InvaderManager, godot::Node)

  /// 現在スコア
  int current_score_ = 0;
  /// 敵がこの Y 座標を超えたら game_over（シーン座標系）
  float game_over_y_ = 520.0f;
  /// 敵が所属するグループ名（シーン側と一致させること）
  godot::String enemy_group_ = "invader_enemies";

  /// game_over / game_clear が一度だけ発行されるよう管理
  bool game_ended_ = false;
  /// 最初の一回をスキップするためのフラグ
  bool initialized_ = false;

  /// ゲーム開始時の敵の総数
  int initial_enemy_count_ = 0;
  /// 基礎移動速度
  float base_move_speed_ = 60.0f;
  /// 最大速度倍率
  float max_speed_multiplier_ = 3.0f;

protected:
  static void _bind_methods();

public:
  InvaderManager();
  ~InvaderManager() override;

  void _process(double delta) override;

  // ------------------------------------------------------------------
  // ゲームロジック API
  // ------------------------------------------------------------------

  /** @brief スコアを加算し、score_changed シグナルを発行する。 */
  void add_score(int points);

  /** @brief プレイヤー被弾を GDScript 側から通知する。game_over をトリガー。 */
  void notify_player_hit();

  /** @brief ゲーム状態をリセットする（リトライ用）。 */
  void reset();

  // ------------------------------------------------------------------
  // プロパティアクセサ
  // ------------------------------------------------------------------
  int get_current_score() const;
  void set_current_score(int v);

  float get_game_over_y() const;
  void set_game_over_y(float v);

  godot::String get_enemy_group() const;
  void set_enemy_group(const godot::String &v);

  float get_base_move_speed() const;
  void set_base_move_speed(float v);

  float get_max_speed_multiplier() const;
  void set_max_speed_multiplier(float v);

private:
  void emit_game_over();
};

} // namespace invaders

#endif // INVADER_MANAGER_H
