#include "invader_barrier.h"
#include "invader_bullet.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace invaders {

void InvaderBarrier::_bind_methods() {
  ClassDB::bind_method(D_METHOD("on_area_entered", "area"),
                       &InvaderBarrier::on_area_entered);

  ClassDB::bind_method(D_METHOD("set_health", "v"),
                       &InvaderBarrier::set_health);
  ClassDB::bind_method(D_METHOD("get_health"), &InvaderBarrier::get_health);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "health"), "set_health",
               "get_health");
}

InvaderBarrier::InvaderBarrier() {}
InvaderBarrier::~InvaderBarrier() {}

void InvaderBarrier::on_area_entered(Area2D *area) {
  // 弾丸（PlayerBullet または単に Area2D に衝突した弾）と衝突
  // ここでは InvaderBullet (Player制) か、Enemy製弾丸かを区別せず、
  // とにかく弾丸らしきもの（後述のグループ指定等でフィルタ可能）であれば耐久を減らす

  // 物体自体の型チェック（InvaderBulletが全弾丸のベースならキャスト可能）
  if (Object::cast_to<InvaderBullet>(area) != nullptr) {
    area->queue_free();
    health_--;
    if (health_ <= 0) {
      queue_free();
    }
  }
}

void InvaderBarrier::set_health(int v) { health_ = v; }
int InvaderBarrier::get_health() const { return health_; }

} // namespace invaders
