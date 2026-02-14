#ifndef HITBOX_COMPONENT_H
#define HITBOX_COMPONENT_H

#include <godot_cpp/classes/area2d.hpp>

using namespace godot;

class HitboxComponent : public Area2D {
  GDCLASS(HitboxComponent, Area2D)

private:
  double damage = 10.0;
  double hit_stun = 0.5;
  Vector2 knockback_force = Vector2(0, 0);

protected:
  static void _bind_methods();
  void _on_area_entered(Area2D *area);

public:
  HitboxComponent();
  ~HitboxComponent();

  // Setters/Getters
  void set_damage(double p_damage);
  double get_damage() const;

  void set_hit_stun(double p_hit_stun);
  double get_hit_stun() const;

  void set_knockback_force(Vector2 p_force);
  Vector2 get_knockback_force() const;
};

#endif // HITBOX_COMPONENT_H
