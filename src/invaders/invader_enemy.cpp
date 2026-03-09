#include "invader_enemy.h"

#include "invader_bullet.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace invaders {

void InvaderEnemy::_bind_methods() {
  // --- シグナル ---
  ADD_SIGNAL(MethodInfo("died",
                        PropertyInfo(Variant::INT, "score_value")));

  // --- コールバック ---
  ClassDB::bind_method(D_METHOD("on_area_entered", "area"),
                       &InvaderEnemy::on_area_entered);

  // --- プロパティ ---
  ClassDB::bind_method(D_METHOD("set_move_speed", "v"),
                       &InvaderEnemy::set_move_speed);
  ClassDB::bind_method(D_METHOD("get_move_speed"),
                       &InvaderEnemy::get_move_speed);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_speed"),
               "set_move_speed", "get_move_speed");

  ClassDB::bind_method(D_METHOD("set_step_down_amount", "v"),
                       &InvaderEnemy::set_step_down_amount);
  ClassDB::bind_method(D_METHOD("get_step_down_amount"),
                       &InvaderEnemy::get_step_down_amount);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "step_down_amount"),
               "set_step_down_amount", "get_step_down_amount");

  ClassDB::bind_method(D_METHOD("set_bounds_half_width", "v"),
                       &InvaderEnemy::set_bounds_half_width);
  ClassDB::bind_method(D_METHOD("get_bounds_half_width"),
                       &InvaderEnemy::get_bounds_half_width);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bounds_half_width"),
               "set_bounds_half_width", "get_bounds_half_width");

  ClassDB::bind_method(D_METHOD("set_score_value", "v"),
                       &InvaderEnemy::set_score_value);
  ClassDB::bind_method(D_METHOD("get_score_value"),
                       &InvaderEnemy::get_score_value);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "score_value"),
               "set_score_value", "get_score_value");
}

InvaderEnemy::InvaderEnemy() {}
InvaderEnemy::~InvaderEnemy() {}

// ------------------------------------------------------------------
// ゲームロジック
// ------------------------------------------------------------------

void InvaderEnemy::_process(double delta) {
  Vector2 pos = get_position();
  pos.x += direction_ * move_speed_ * static_cast<float>(delta);

  // 端に達したら一段下がり方向を反転
  if (pos.x >  bounds_half_width_) {
    pos.x        =  bounds_half_width_;
    pos.y       += step_down_amount_;
    direction_   = -1.0f;
  } else if (pos.x < -bounds_half_width_) {
    pos.x        = -bounds_half_width_;
    pos.y       += step_down_amount_;
    direction_   =  1.0f;
  }

  set_position(pos);
}

void InvaderEnemy::on_area_entered(Area2D *area) {
  // InvaderBullet と衝突したときだけ died を発行する
  if (Object::cast_to<InvaderBullet>(area) == nullptr) {
    return;
  }

  // 弾を消す
  area->queue_free();

  emit_signal("died", score_value_);
  queue_free();
}

// ------------------------------------------------------------------
// プロパティアクセサ
// ------------------------------------------------------------------

void  InvaderEnemy::set_move_speed(float v)        { move_speed_        = v; }
float InvaderEnemy::get_move_speed() const         { return move_speed_;    }

void  InvaderEnemy::set_step_down_amount(float v)  { step_down_amount_  = v; }
float InvaderEnemy::get_step_down_amount() const   { return step_down_amount_; }

void  InvaderEnemy::set_bounds_half_width(float v) { bounds_half_width_ = v; }
float InvaderEnemy::get_bounds_half_width() const  { return bounds_half_width_; }

void InvaderEnemy::set_score_value(int v)          { score_value_       = v; }
int  InvaderEnemy::get_score_value() const         { return score_value_;   }

} // namespace invaders
