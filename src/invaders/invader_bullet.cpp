#include "invader_bullet.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace invaders {

void InvaderBullet::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_speed", "speed"),
                       &InvaderBullet::set_speed);
  ClassDB::bind_method(D_METHOD("get_speed"), &InvaderBullet::get_speed);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

  ClassDB::bind_method(D_METHOD("set_direction", "direction"),
                       &InvaderBullet::set_direction);
  ClassDB::bind_method(D_METHOD("get_direction"),
                       &InvaderBullet::get_direction);
  ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "direction"), "set_direction",
               "get_direction");
}

InvaderBullet::InvaderBullet() {}
InvaderBullet::~InvaderBullet() {}

void InvaderBullet::_physics_process(double delta) {
  Vector2 pos = get_position();
  pos += direction_ * speed_ * (float)delta;
  set_position(pos);

  if (pos.y < -50.0f || pos.y > 650.0f) {
    queue_free();
  }
}

void InvaderBullet::set_speed(float p_speed) { speed_ = p_speed; }
float InvaderBullet::get_speed() const { return speed_; }

void InvaderBullet::set_direction(const Vector2 &p_direction) {
  direction_ = p_direction.normalized();
}
Vector2 InvaderBullet::get_direction() const { return direction_; }

} // namespace invaders
