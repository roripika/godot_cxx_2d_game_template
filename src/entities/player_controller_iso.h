#ifndef PLAYER_CONTROLLER_ISO_H
#define PLAYER_CONTROLLER_ISO_H

#include "game_entity.h"
#include <godot_cpp/classes/input.hpp>

using namespace godot;

class PlayerControllerIso : public GameEntity {
  GDCLASS(PlayerControllerIso, GameEntity)

protected:
  static void _bind_methods();
  void _notification(int p_what);

public:
  PlayerControllerIso();
  ~PlayerControllerIso();

  // Override to apply specialized Isometric movement
  virtual void apply_movement(double delta) override;
};

#endif
