#include "game_entity.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GameEntity::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_max_health", "health"),
                       &GameEntity::set_max_health);
  ClassDB::bind_method(D_METHOD("get_max_health"), &GameEntity::get_max_health);
  ClassDB::bind_method(D_METHOD("set_current_health", "health"),
                       &GameEntity::set_current_health);
  ClassDB::bind_method(D_METHOD("get_current_health"),
                       &GameEntity::get_current_health);
  ClassDB::bind_method(D_METHOD("set_speed", "speed"), &GameEntity::set_speed);
  ClassDB::bind_method(D_METHOD("get_speed"), &GameEntity::get_speed);

  ClassDB::bind_method(D_METHOD("take_damage", "amount"),
                       &GameEntity::take_damage);
  ClassDB::bind_method(D_METHOD("heal", "amount"), &GameEntity::heal);
  ClassDB::bind_method(D_METHOD("set_movement_input", "input"),
                       &GameEntity::set_movement_input);
  ClassDB::bind_method(D_METHOD("get_movement_input"),
                       &GameEntity::get_movement_input);

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_health"), "set_max_health",
               "get_max_health");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "current_health"),
               "set_current_health", "get_current_health");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

  ADD_SIGNAL(
      MethodInfo("health_changed", PropertyInfo(Variant::FLOAT, "new_health")));
  ADD_SIGNAL(MethodInfo("died"));
}

GameEntity::GameEntity() {
  max_health = 100.0f;
  current_health = 100.0f;
  speed = 200.0f;
  movement_input = Vector2(0, 0);
}

GameEntity::~GameEntity() {}

void GameEntity::_notification(int p_what) {
  if (p_what == NOTIFICATION_PHYSICS_PROCESS) {
    if (is_physics_processing()) {
      apply_movement(get_physics_process_delta_time());
    }
  }
}

void GameEntity::set_max_health(float p_health) {
  max_health = p_health;
  if (current_health > max_health) {
    current_health = max_health;
    emit_signal("health_changed", current_health);
  }
}

float GameEntity::get_max_health() const { return max_health; }

void GameEntity::set_current_health(float p_health) {
  current_health = p_health;
  emit_signal("health_changed", current_health);
  if (current_health <= 0) {
    emit_signal("died");
  }
}

float GameEntity::get_current_health() const { return current_health; }

void GameEntity::set_speed(float p_speed) { speed = p_speed; }

float GameEntity::get_speed() const { return speed; }

void GameEntity::take_damage(float p_amount) {
  current_health -= p_amount;
  emit_signal("health_changed", current_health);
  if (current_health <= 0) {
    emit_signal("died");
  }
}

void GameEntity::heal(float p_amount) {
  current_health += p_amount;
  if (current_health > max_health) {
    current_health = max_health;
  }
  emit_signal("health_changed", current_health);
}

void GameEntity::set_movement_input(const Vector2 &p_input) {
  movement_input = p_input;
}

Vector2 GameEntity::get_movement_input() const { return movement_input; }

void GameEntity::apply_movement(double delta) {
  Vector2 target_velocity = movement_input * speed;
  set_velocity(target_velocity);
  move_and_slide();
}
