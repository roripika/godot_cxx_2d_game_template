#include "adventure_game_state.h"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

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
  ClassDB::bind_method(D_METHOD("set_flag", "key", "value"),
                       &AdventureGameStateBase::set_flag);
  ClassDB::bind_method(D_METHOD("get_flag", "key", "default_value"),
                       &AdventureGameStateBase::get_flag, DEFVAL(false));

  ClassDB::bind_method(D_METHOD("add_item", "item_name"),
                       &AdventureGameStateBase::add_item);
  ClassDB::bind_method(D_METHOD("remove_item", "item_name"),
                       &AdventureGameStateBase::remove_item);
  ClassDB::bind_method(D_METHOD("has_item", "item_name"),
                       &AdventureGameStateBase::has_item);

  ClassDB::bind_method(D_METHOD("change_scene", "path"),
                       &AdventureGameStateBase::change_scene);
  
  // HP management
  ClassDB::bind_method(D_METHOD("set_health", "hp"),
                       &AdventureGameStateBase::set_health);
  ClassDB::bind_method(D_METHOD("get_health"),
                       &AdventureGameStateBase::get_health);
  ClassDB::bind_method(D_METHOD("take_damage"),
                       &AdventureGameStateBase::take_damage);
  ClassDB::bind_method(D_METHOD("heal", "amount"),
                       &AdventureGameStateBase::heal);
  ADD_SIGNAL(MethodInfo("health_changed", PropertyInfo(Variant::INT, "hp")));
  
  // Game reset
  ClassDB::bind_method(D_METHOD("reset_game"),
                       &AdventureGameStateBase::reset_game);
}

void AdventureGameStateBase::set_flag(const String &key, bool value) {
  flags[key] = value;
}

bool AdventureGameStateBase::get_flag(const String &key,
                                      bool default_value) const {
  if (flags.has(key)) {
    return flags[key];
  }
  return default_value;
}

void AdventureGameStateBase::add_item(const String &item_name) {
  if (!has_item(item_name)) {
    inventory.append(item_name);
    UtilityFunctions::print("Item added: ", item_name);
  }
}

void AdventureGameStateBase::remove_item(const String &item_name) {
  int index = inventory.find(item_name);
  if (index != -1) {
    inventory.remove_at(index);
  }
}

bool AdventureGameStateBase::has_item(const String &item_name) const {
  return inventory.has(item_name);
}

void AdventureGameStateBase::change_scene(const String &path) {
  UtilityFunctions::print("Changing scene to: ", path);
  SceneTree *tree = get_tree();
  if (tree) {
    tree->change_scene_to_file(path);
  }
}

void AdventureGameStateBase::set_health(int hp) {
  health = hp;
  UtilityFunctions::print("Health set to: ", health);
  emit_signal("health_changed", health);
}

int AdventureGameStateBase::get_health() const {
  return health;
}

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
  flags.clear();
  inventory.clear();
  health = 3;
  UtilityFunctions::print("Game reset. Health: ", health);
  emit_signal("health_changed", health);
}
