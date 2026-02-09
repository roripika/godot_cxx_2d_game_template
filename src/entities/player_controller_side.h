#ifndef PLAYER_CONTROLLER_SIDE_H
#define PLAYER_CONTROLLER_SIDE_H

#include "game_entity.h"
#include <godot_cpp/classes/input.hpp>

using namespace godot;

class PlayerControllerSide : public GameEntity {
  GDCLASS(PlayerControllerSide, GameEntity)

private:
  float jump_force;
  float gravity;

protected:
  static void _bind_methods();
  void _notification(int p_what);

public:
  PlayerControllerSide();
  ~PlayerControllerSide();

  void set_jump_force(float p_force);
  float get_jump_force() const;

  void set_gravity(float p_gravity);
  float get_gravity() const;

  // Override to apply Platformer movement (Gravity + Jump)
  virtual void apply_movement(double delta) override;
};

#endif
