#ifndef FIGHTER_CONTROLLER_H
#define FIGHTER_CONTROLLER_H

// Stub: FighterController will be fully implemented in src/plugins/features/fighting/
// when STEP 3 of layer_migration_plan.md is executed.

#include <godot_cpp/classes/character_body2d.hpp>

using namespace godot;

class FighterController : public CharacterBody2D {
  GDCLASS(FighterController, CharacterBody2D)

protected:
  static void _bind_methods() {}
};

#endif // FIGHTER_CONTROLLER_H
