#include "player_controller_side.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PlayerControllerSide::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_jump_force", "force"),
                       &PlayerControllerSide::set_jump_force);
  ClassDB::bind_method(D_METHOD("get_jump_force"),
                       &PlayerControllerSide::get_jump_force);
  ClassDB::bind_method(D_METHOD("set_gravity", "gravity"),
                       &PlayerControllerSide::set_gravity);
  ClassDB::bind_method(D_METHOD("get_gravity"),
                       &PlayerControllerSide::get_gravity);

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_force"), "set_jump_force",
               "get_jump_force");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity",
               "get_gravity");
}

// Accessors
void PlayerControllerSide::set_jump_force(float p_force) {
  jump_force = p_force;
}

float PlayerControllerSide::get_jump_force() const { return jump_force; }

void PlayerControllerSide::set_gravity(float p_gravity) { gravity = p_gravity; }

float PlayerControllerSide::get_gravity() const { return gravity; }

PlayerControllerSide::PlayerControllerSide() {
  set_speed(200.0f);
  jump_force = 400.0f;
  gravity = 980.0f; // Standard pixel gravity
}

PlayerControllerSide::~PlayerControllerSide() {}

void PlayerControllerSide::_notification(int p_what) {
  if (p_what == NOTIFICATION_PHYSICS_PROCESS) {
    if (is_physics_processing()) {
      double delta = get_physics_process_delta_time();
      Input *input = Input::get_singleton();

      // Horizontal Input
      Vector2 raw_input(0, 0);
      raw_input.x = input->get_axis("ui_left", "ui_right");
      set_movement_input(raw_input);

      // Jump Input handling in apply_movement or here?
      // Let's handle it here to set velocity, but apply_movement does the
      // slide. Actually, we need to modify direct velocity for Jump.

      if (input->is_action_just_pressed("ui_accept") && is_on_floor()) {
        Vector2 vel = get_velocity();
        vel.y = -jump_force;
        set_velocity(vel);
      }

      apply_movement(delta);
    }
  }
}

void PlayerControllerSide::apply_movement(double delta) {
  Vector2 velocity = get_velocity();
  Vector2 input = get_movement_input();
  float speed = get_speed();

  // Apply Gravity
  if (!is_on_floor()) {
    velocity.y += gravity * delta;
  }

  // Apply Horizontal Movement
  if (input.x != 0) {
    velocity.x = input.x * speed;
  } else {
    // Simple friction/stop
    velocity.x = UtilityFunctions::move_toward(velocity.x, 0, speed);
  }

  set_velocity(velocity);
  move_and_slide();
}
