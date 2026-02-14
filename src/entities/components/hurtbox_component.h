#ifndef HURTBOX_COMPONENT_H
#define HURTBOX_COMPONENT_H

#include "hitbox_component.h"
#include <godot_cpp/classes/area2d.hpp>

using namespace godot;

class HurtboxComponent : public Area2D {
  GDCLASS(HurtboxComponent, Area2D)

private:
  // Optional: Reference to HealthComponent or Parent Entity
  bool invincible = false;

protected:
  static void _bind_methods();

public:
  HurtboxComponent();
  ~HurtboxComponent();

  // Logic
  void take_hit(HitboxComponent *hitbox);

  void set_invincible(bool p_invincible);
  bool is_invincible() const;
};

#endif // HURTBOX_COMPONENT_H
