#include "hurtbox_component.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

HurtboxComponent::HurtboxComponent() {
  set_monitoring(false);
  set_monitorable(true);
}

HurtboxComponent::~HurtboxComponent() {}

void HurtboxComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("take_hit", "hitbox_component"),
                       &HurtboxComponent::take_hit);
  ClassDB::bind_method(D_METHOD("set_invincible", "invincible"),
                       &HurtboxComponent::set_invincible);
  ClassDB::bind_method(D_METHOD("is_invincible"),
                       &HurtboxComponent::is_invincible);

  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "invincible"), "set_invincible",
               "is_invincible");

  ADD_SIGNAL(MethodInfo("taken_hit", PropertyInfo(Variant::OBJECT, "hitbox"),
                        PropertyInfo(Variant::FLOAT, "damage"),
                        PropertyInfo(Variant::FLOAT, "hit_stun"),
                        PropertyInfo(Variant::VECTOR2, "knockback_force")));
}

void HurtboxComponent::take_hit(HitboxComponent *hitbox) {
  if (invincible || !hitbox)
    return;

  double damage = hitbox->get_damage();
  double hit_stun = hitbox->get_hit_stun();
  Vector2 knockback = hitbox->get_knockback_force();

  // UtilityFunctions::print("Hurtbox took hit! Damage: ", damage);

  // Emit signal so parent entity can handle health reduction, animation, etc.
  // Note: We pass the hitbox object itself as well, in case more data is
  // needed.
  emit_signal("taken_hit", hitbox, damage, hit_stun, knockback);
}

void HurtboxComponent::set_invincible(bool p_invincible) {
  invincible = p_invincible;
}
bool HurtboxComponent::is_invincible() const { return invincible; }
