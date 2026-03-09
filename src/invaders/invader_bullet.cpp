#include "invader_bullet.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace invaders {

void InvaderBullet::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_speed", "speed"),
                       &InvaderBullet::set_speed);
  ClassDB::bind_method(D_METHOD("get_speed"), &InvaderBullet::get_speed);

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");
}

InvaderBullet::InvaderBullet() {}
InvaderBullet::~InvaderBullet() {}

void InvaderBullet::_physics_process(double delta) {
  Vector2 pos = get_position();
  pos.y += speed * (float)delta;
  set_position(pos);

  if (pos.y < -50.0f) {
    queue_free();
  }
}

void InvaderBullet::set_speed(float p_speed) { speed = p_speed; }

float InvaderBullet::get_speed() const { return speed; }

} // namespace invaders
