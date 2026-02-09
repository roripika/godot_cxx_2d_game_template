#include "player_controller_iso.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PlayerControllerIso::_bind_methods() {
  // No new methods to bind yet, just using inherited ones.
}

PlayerControllerIso::PlayerControllerIso() {
  // Default higher speed for player
  set_speed(300.0f);
}

PlayerControllerIso::~PlayerControllerIso() {}

void PlayerControllerIso::_notification(int p_what) {
  if (p_what == NOTIFICATION_PHYSICS_PROCESS) {
    if (is_physics_processing()) {
      // Get Input directly here or allow it to be pushed.
      // For a Controller class, it's common to poll Input here.
      Input *input = Input::get_singleton();
      Vector2 raw_input =
          input->get_vector("ui_left", "ui_right", "ui_up", "ui_down");

      // Standard Top-Down logic:
      // In a pure 2D top-down game, Up is Y-.
      // In Isometric, visual "Up" is usually Up-Right or Up-Left depending on
      // projection.
      //
      // Godot's Isometric TileMap default:
      // X-axis: Down-Right
      // Y-axis: Down-Left
      //
      // If we want "Up" key to move visually Up (North):
      // Visual Up is -Y in screen space.
      // To achieve -Y screen space in Iso projection (where y = (x+y)/2), we
      // need x-y to decrease?
      //
      // Let's stick to Cartesian input for movement vector first (Up = Y-),
      // and simply apply velocity. The physics engine handles the rest.
      // If the user wants "Up" to mean "North-West" (Iso Axis Y-), they press
      // Up+Left?
      //
      // Standard approach for 2D Iso action games:
      // Up Key -> Moves visually Up (Screen Y-) -> Requires specific Vector
      // math if TileMap is Iso. But GameEntity relies on `move_and_slide` which
      // is screen-space pixel coordinates.
      //
      // So, actually:
      // Input "Up" -> Vector2(0, -1).
      // This moves the character straight UP on the screen.
      // In an Isometric game, this is usually what players expect
      // (screen-relative movement).
      //
      // If we wanted "Tank Controls" relative to the grid, we'd map:
      // Up -> Grid(0, -1) -> Screen(?, ?)

      // For this template, we implemented Screen-Relative movement (Standard
      // Action RPG style). So logic remains simple, effectively same as
      // top-down. We optimize by just overriding to normalize input.

      if (raw_input.length_squared() > 1.0) {
        raw_input = raw_input.normalized();
      }

      set_movement_input(raw_input);
      apply_movement(get_physics_process_delta_time());
    }
  }
}

void PlayerControllerIso::apply_movement(double delta) {
  // We can add isometric specific collision handling or snap-to-grid here if
  // needed later. For now, base implementation is sufficient for free movement.
  GameEntity::apply_movement(delta);
}
