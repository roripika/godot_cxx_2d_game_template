#ifndef INTERACTION_MANAGER_H
#define INTERACTION_MANAGER_H

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

using namespace godot;

class InteractionManager : public Node2D {
  GDCLASS(InteractionManager, Node2D)

private:
  bool active;
  Vector2 cursor_position;

protected:
  static void _bind_methods();

public:
  InteractionManager();
  ~InteractionManager();

  void _input(const Ref<InputEvent> &p_event) override;

  // Signals
  // emitted when a "clickable" area/object is clicked
  // signal clicked_at(position)

  void set_active(bool p_active);
  bool is_active() const;
};

#endif
