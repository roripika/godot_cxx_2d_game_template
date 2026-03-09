#ifndef INVADER_ENEMY_H
#define INVADER_ENEMY_H

/**
 * @file invader_enemy.h
 * @brief インベーダーゲーム: 敵クラス。
 *
 * ## 動作
 * - 左右に一定速度で移動し、画面端（±bounds_half_width）に達したら
 *   一段下がって方向を反転する（インベーダー特有の動き）。
 *
 * ## シグナル
 * - `died(int score_value)` — InvaderBullet と衝突した際に発行。
 *   InvaderManager がこれを受け取りスコアを加算する。
 *
 * ## UI 結線ルール
 * C++ 内にノード操作を書かない。UI 更新は GDScript View 層が担当する。
 */

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace invaders {

class InvaderEnemy : public godot::Area2D {
  GDCLASS(InvaderEnemy, godot::Area2D)

  /// 水平移動速度 (px/s)
  float move_speed_ = 60.0f;
  /// 端に達したときの下降量 (px)
  float step_down_amount_ = 24.0f;
  /// 画面中心からの移動可能半幅 (px)。超えたら反転＋下降。
  float bounds_half_width_ = 200.0f;
  /// 撃破時のスコア加算値
  int score_value_ = 10;

  /// 現在の水平移動方向（+1 or -1）
  float direction_ = 1.0f;

  /// 射撃間隔 (秒)
  float shoot_interval_min_ = 2.0f;
  float shoot_interval_max_ = 10.0f;
  /// 次の射撃までの残り時間
  float shoot_timer_ = 0.0f;

  void reset_shoot_timer();

protected:
  static void _bind_methods();

public:
  InvaderEnemy();
  ~InvaderEnemy() override;

  void _process(double delta) override;

  /// InvaderBullet との衝突コールバック（シーン側で Area2D.area_entered
  /// に接続）
  void on_area_entered(godot::Area2D *area);

  // ------------------------------------------------------------------
  // プロパティアクセサ
  // ------------------------------------------------------------------
  void set_move_speed(float v);
  float get_move_speed() const;

  void set_step_down_amount(float v);
  float get_step_down_amount() const;

  void set_bounds_half_width(float v);
  float get_bounds_half_width() const;

  void set_score_value(int v);
  int get_score_value() const;

  void set_shoot_interval_min(float v);
  float get_shoot_interval_min() const;

  void set_shoot_interval_max(float v);
  float get_shoot_interval_max() const;
};

} // namespace invaders

#endif // INVADER_ENEMY_H
