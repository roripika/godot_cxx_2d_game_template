#include "raycast_component.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

RayCastComponent::RayCastComponent() {}
RayCastComponent::~RayCastComponent() {}

void RayCastComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("is_colliding_with_type", "class_name"),
                       &RayCastComponent::is_colliding_with_type);
}

bool RayCastComponent::is_colliding_with_type(String class_name) {
  if (!is_colliding())
    return false;

  Object *collider = get_collider();
  if (!collider)
    return false;

  return collider->is_class(class_name);
}
