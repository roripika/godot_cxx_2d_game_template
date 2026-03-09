#include "invader_player.h"
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace invaders {

void InvaderPlayer::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_move_speed", "speed"),
                       &InvaderPlayer::set_move_speed);
  ClassDB::bind_method(D_METHOD("get_move_speed"),
                       &InvaderPlayer::get_move_speed);
  ClassDB::bind_method(D_METHOD("shoot"), &InvaderPlayer::shoot);

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_speed"), "set_move_speed",
               "get_move_speed");

  ADD_SIGNAL(MethodInfo("bullet_fired",
                        PropertyInfo(Variant::VECTOR2, "spawn_position")));
}

InvaderPlayer::InvaderPlayer() {}
InvaderPlayer::~InvaderPlayer() {}

void InvaderPlayer::_physics_process(double delta) {
  Input *input = Input::get_singleton();
  Vector2 direction(0, 0);

  if (input->is_action_pressed("ui_left")) {
    direction.x -= 1.0;
  }
  if (input->is_action_pressed("ui_right")) {
    direction.x += 1.0;
  }

  Vector2 pos = get_position();
  pos += direction * move_speed * (float)delta;
  set_position(pos);

  if (input->is_action_just_pressed("ui_accept")) {
    shoot();
  }
}

void InvaderPlayer::shoot() { emit_signal("bullet_fired", get_position()); }

void InvaderPlayer::set_move_speed(float p_speed) { move_speed = p_speed; }

float InvaderPlayer::get_move_speed() const { return move_speed; }

} // namespace invaders
