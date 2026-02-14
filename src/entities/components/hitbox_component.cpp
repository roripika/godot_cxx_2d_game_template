#include "hitbox_component.h"
#include "hurtbox_component.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

HitboxComponent::HitboxComponent() {
  set_monitoring(true);
  set_monitorable(false);
  connect("area_entered", Callable(this, "_on_area_entered"));
}

HitboxComponent::~HitboxComponent() {}

void HitboxComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_damage", "damage"),
                       &HitboxComponent::set_damage);
  ClassDB::bind_method(D_METHOD("get_damage"), &HitboxComponent::get_damage);
  ClassDB::bind_method(D_METHOD("set_hit_stun", "hit_stun"),
                       &HitboxComponent::set_hit_stun);
  ClassDB::bind_method(D_METHOD("get_hit_stun"),
                       &HitboxComponent::get_hit_stun);
  ClassDB::bind_method(D_METHOD("set_knockback_force", "force"),
                       &HitboxComponent::set_knockback_force);
  ClassDB::bind_method(D_METHOD("get_knockback_force"),
                       &HitboxComponent::get_knockback_force);

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "damage"), "set_damage",
               "get_damage");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "hit_stun"), "set_hit_stun",
               "get_hit_stun");
  ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "knockback_force"),
               "set_knockback_force", "get_knockback_force");

  ADD_SIGNAL(
      MethodInfo("hit_connected", PropertyInfo(Variant::OBJECT, "hurtbox")));

  ClassDB::bind_method(D_METHOD("_on_area_entered", "area"),
                       &HitboxComponent::_on_area_entered);
}

void HitboxComponent::set_damage(double p_damage) { damage = p_damage; }
double HitboxComponent::get_damage() const { return damage; }

void HitboxComponent::set_hit_stun(double p_hit_stun) { hit_stun = p_hit_stun; }
double HitboxComponent::get_hit_stun() const { return hit_stun; }

void HitboxComponent::set_knockback_force(Vector2 p_force) {
  knockback_force = p_force;
}
Vector2 HitboxComponent::get_knockback_force() const { return knockback_force; }

void HitboxComponent::_on_area_entered(Area2D *area) {
  HurtboxComponent *hurtbox = Object::cast_to<HurtboxComponent>(area);
  if (hurtbox) {
    hurtbox->take_hit(this);
    emit_signal("hit_connected", hurtbox);
  }
}
