#include "adventure_game_state.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {


AdventureGameStateBase *AdventureGameStateBase::singleton = nullptr;

AdventureGameStateBase::AdventureGameStateBase() {
  if (singleton == nullptr) {
    singleton = this;
  }
}

AdventureGameStateBase::~AdventureGameStateBase() {
  if (singleton == this) {
    singleton = nullptr;
  }
}

AdventureGameStateBase *AdventureGameStateBase::get_singleton() {
  return singleton;
}

void AdventureGameStateBase::_bind_methods() {
  // HP 管理
  ClassDB::bind_method(D_METHOD("set_health", "hp"),
                       &AdventureGameStateBase::set_health);
  ClassDB::bind_method(D_METHOD("get_health"),
                       &AdventureGameStateBase::get_health);
  ClassDB::bind_method(D_METHOD("take_damage"),
                       &AdventureGameStateBase::take_damage);
  ClassDB::bind_method(D_METHOD("heal", "amount"),
                       &AdventureGameStateBase::heal);
  ADD_SIGNAL(MethodInfo("health_changed", PropertyInfo(Variant::INT, "hp")));

  // ゲームリセット
  ClassDB::bind_method(D_METHOD("reset_game"),
                       &AdventureGameStateBase::reset_game);
  ClassDB::bind_method(D_METHOD("set_reset_hook", "hook"),
                       &AdventureGameStateBase::set_reset_hook);
}

void AdventureGameStateBase::set_health(int hp) {
  health = hp;
  UtilityFunctions::print("Health set to: ", health);
  emit_signal("health_changed", health);
}

int AdventureGameStateBase::get_health() const { return health; }

void AdventureGameStateBase::take_damage() {
  if (health > 0) {
    health--;
    UtilityFunctions::print("Took damage. Health now: ", health);
    emit_signal("health_changed", health);
  }
}

void AdventureGameStateBase::heal(int amount) {
  health += amount;
  UtilityFunctions::print("Healed by ", amount, ". Health now: ", health);
  emit_signal("health_changed", health);
}

void AdventureGameStateBase::reset_game() {
  // Mystery 層が登録したフックがあれば呼び出す（EvidenceManager/MysteryManagerのクリア等）
  if (reset_hook_.is_valid()) {
    reset_hook_.call();
  }
  health = 3;
  UtilityFunctions::print("Game reset. Health: ", health);
  emit_signal("health_changed", health);
}

void AdventureGameStateBase::set_reset_hook(const godot::Callable &hook) {
  reset_hook_ = hook;
}

} // namespace karakuri
