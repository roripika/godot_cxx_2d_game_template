#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include <godot_cpp/classes/character_body2d.hpp>

using namespace godot;

class GameEntity : public CharacterBody2D {
  GDCLASS(GameEntity, CharacterBody2D)

private:
  float max_health;
  float current_health;
  float speed;

  // Abstract input direction (to be set by controller logic)
  Vector2 movement_input;

protected:
  static void _bind_methods();
  void _notification(int p_what);

public:
  GameEntity();
  ~GameEntity();

  // Stats
  void set_max_health(float p_health);
  float get_max_health() const;

  void set_current_health(float p_health);
  float get_current_health() const;

  void set_speed(float p_speed);
  float get_speed() const;

  // Actions
  void take_damage(float p_amount);
  void heal(float p_amount);

  // Movement Control
  void set_movement_input(const Vector2 &p_input);
  Vector2 get_movement_input() const;

  // Simple movement logic (can be overridden or handled by components)
  // Applies velocity based on input * speed and calls move_and_slide()
  virtual void apply_movement(double delta);
};

#endif
