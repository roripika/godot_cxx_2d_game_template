#include "mystery_game_master.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

MysteryGameMaster *MysteryGameMaster::singleton = nullptr;

MysteryGameMaster::MysteryGameMaster() {
  ERR_FAIL_COND(singleton != nullptr);
  singleton = this;
}

MysteryGameMaster::~MysteryGameMaster() {
  ERR_FAIL_COND(singleton != this);
  singleton = nullptr;
}

MysteryGameMaster *MysteryGameMaster::get_singleton() { return singleton; }

void MysteryGameMaster::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_flag", "p_name", "p_value"),
                       &MysteryGameMaster::set_flag);
  ClassDB::bind_method(D_METHOD("get_flag", "p_name"),
                       &MysteryGameMaster::get_flag);
  ClassDB::bind_method(D_METHOD("serialize_flags"),
                       &MysteryGameMaster::serialize_flags);
  ClassDB::bind_method(D_METHOD("deserialize_flags", "p_dict"),
                       &MysteryGameMaster::deserialize_flags);
}

void MysteryGameMaster::set_flag(const String &p_name, bool p_value) {
  // Check if it already exists and if it's changing
  bool old_value = get_flag(p_name);
  if (flags.find(p_name) == flags.end() || old_value != p_value) {
    flags[p_name] = p_value;

    // Output helpful log for debugging regression
    UtilityFunctions::print("[MysteryGameMaster] Flag Changed: '", p_name,
                            "' from ", old_value ? "true" : "false", " to ",
                            p_value ? "true" : "false");
  }
}

bool MysteryGameMaster::get_flag(const String &p_name) const {
  auto it = flags.find(p_name);
  if (it != flags.end()) {
    return it->second;
  }
  return false; // Default flag status is false
}

Dictionary MysteryGameMaster::serialize_flags() const {
  Dictionary dict;
  for (const auto &pair : flags) {
    dict[pair.first] = pair.second;
  }
  return dict;
}

void MysteryGameMaster::deserialize_flags(const Dictionary &p_dict) {
  flags.clear();
  Array keys = p_dict.keys();
  for (int i = 0; i < keys.size(); i++) {
    String key = keys[i];
    bool value = p_dict[key];
    flags[key] = value;
  }
  UtilityFunctions::print("[MysteryGameMaster] Flags Deserialized. Count: ",
                          (int64_t)flags.size());
}
