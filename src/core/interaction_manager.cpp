#include "interaction_manager.h"
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void InteractionManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_active", "active"),
                       &InteractionManager::set_active);
  ClassDB::bind_method(D_METHOD("is_active"), &InteractionManager::is_active);

  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active",
               "is_active");

  ADD_SIGNAL(
      MethodInfo("clicked_at", PropertyInfo(Variant::VECTOR2, "position")));
}

InteractionManager::InteractionManager() { active = true; }

InteractionManager::~InteractionManager() {}

void InteractionManager::_input(const Ref<InputEvent> &p_event) {
  if (!active)
    return;

  Ref<InputEventMouseMotion> motion_event = p_event;
  if (motion_event.is_valid()) {
    cursor_position = get_global_mouse_position();
  }

  Ref<InputEventMouseButton> mouse_event = p_event;
  if (mouse_event.is_valid()) {
    if (mouse_event->is_pressed() &&
        mouse_event->get_button_index() == MOUSE_BUTTON_LEFT) {
      // Emit Canvas/World coordinates so listeners can compare directly with
      // Area2D global positions even when camera/viewport transforms change.
      const Vector2 canvas_pos = get_global_mouse_position();
      cursor_position = canvas_pos;
      emit_signal("clicked_at", canvas_pos);
      // TODO: Raycast logic here if needed, or rely on Area2D inputs elsewhere
      // handled by this signal? "Urban Myth Dissolution Center" style ->
      // usually simpler point & click.
    }
  }
}

void InteractionManager::set_active(bool p_active) {
  active = p_active;
  set_process_input(active);
}

bool InteractionManager::is_active() const { return active; }
