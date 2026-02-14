#ifndef RAYCAST_COMPONENT_H
#define RAYCAST_COMPONENT_H

#include <godot_cpp/classes/ray_cast2d.hpp>

using namespace godot;

class RayCastComponent : public RayCast2D {
  GDCLASS(RayCastComponent, RayCast2D)

protected:
  static void _bind_methods();

public:
  RayCastComponent();
  ~RayCastComponent();

  // Helper method: Is colliding with specific class or group?
  bool is_colliding_with_type(String class_name);
};

#endif // RAYCAST_COMPONENT_H
