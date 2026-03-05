#include "mystery_manager.h"
#include "../core/adventure_game_state.h"
#include "evidence_manager.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

MysteryManager *MysteryManager::singleton = nullptr;

void MysteryManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_flag", "name", "value"),
                       &MysteryManager::set_flag);
  ClassDB::bind_method(D_METHOD("get_flag", "name"), &MysteryManager::get_flag);
  ClassDB::bind_method(D_METHOD("serialize_state"),
                       &MysteryManager::serialize_state);
  ClassDB::bind_method(D_METHOD("deserialize_state", "dict"),
                       &MysteryManager::deserialize_state);
  ClassDB::bind_method(D_METHOD("deserialize_flags", "dict"),
                       &MysteryManager::deserialize_flags);
  ClassDB::bind_method(D_METHOD("save_checkpoint", "scene_path"),
                       &MysteryManager::save_checkpoint);
  ClassDB::bind_method(D_METHOD("load_checkpoint"),
                       &MysteryManager::load_checkpoint);
}

MysteryManager::MysteryManager() {
  if (singleton == nullptr) {
    singleton = this;
  }
}

MysteryManager::~MysteryManager() {
  if (singleton == this) {
    singleton = nullptr;
  }
}

MysteryManager *MysteryManager::get_singleton() { return singleton; }

void MysteryManager::set_mystery_flag(MysteryFlag p_flag, bool p_value) {
  set_flag(MysteryEnumValueConverter::get_flag_key(p_flag), p_value);
}

bool MysteryManager::get_mystery_flag(MysteryFlag p_flag) const {
  return get_flag(MysteryEnumValueConverter::get_flag_key(p_flag));
}

void MysteryManager::set_flag(const String &p_name, bool p_value) {
  bool old_value = false;
  if (flags.count(p_name)) {
    old_value = flags[p_name];
  }

  if (old_value != p_value) {
    flags[p_name] = p_value;
    log_change("Flag", p_name, old_value, p_value, "set_flag");
    validate_state();
  }
}

bool MysteryManager::get_flag(const String &p_name) const {
  if (flags.count(p_name)) {
    return flags.at(p_name);
  }
  return false;
}

Dictionary MysteryManager::serialize_state() const {
  Dictionary dict;
  Dictionary flags_dict;
  for (const auto &pair : flags) {
    flags_dict[pair.first] = pair.second;
  }
  dict["flags"] = flags_dict;

  auto *em = EvidenceManager::get_singleton();
  if (em) {
    dict["evidence"] = em->serialize_evidence();
  }

  auto *ags = karakuri::AdventureGameStateBase::get_singleton();
  if (ags) {
    dict["health"] = ags->get_health();
  }

  return dict;
}

void MysteryManager::deserialize_state(const Dictionary &p_dict) {
  if (p_dict.has("flags")) {
    deserialize_flags(p_dict["flags"]);
  }

  if (p_dict.has("evidence")) {
    auto *em = EvidenceManager::get_singleton();
    if (em) {
      em->deserialize_evidence(p_dict["evidence"]);
    }
  }

  if (p_dict.has("health")) {
    auto *ags = karakuri::AdventureGameStateBase::get_singleton();
    if (ags) {
      ags->set_health(p_dict["health"]);
    }
  }
}

void MysteryManager::deserialize_flags(const Dictionary &p_dict) {
  Array keys = p_dict.keys();
  flags.clear();
  for (int i = 0; i < keys.size(); i++) {
    String key = keys[i];
    flags[key] = p_dict[key];
  }
  UtilityFunctions::print("[LOG] Flags: ALL | Old: VARIOUS | New: "
                          "DESERIALIZED | Reason: deserialize_flags");
  validate_state();
}

void MysteryManager::save_checkpoint(const String &p_scene_path) {
  checkpoint_scene = p_scene_path;
  Dictionary state = serialize_state();
  checkpoint_flags = state["flags"];
  checkpoint_evidence = state["evidence"];
  checkpoint_health = state["health"];
  has_checkpoint = true;
  UtilityFunctions::print("[CHECKPOINT] Saved at scene: ", p_scene_path);
}

String MysteryManager::load_checkpoint() {
  if (!has_checkpoint) {
    UtilityFunctions::print("[CHECKPOINT] No checkpoint to load!");
    return "";
  }

  Dictionary state;
  state["flags"] = checkpoint_flags;
  state["evidence"] = checkpoint_evidence;
  state["health"] = checkpoint_health;
  deserialize_state(state);

  UtilityFunctions::print("[CHECKPOINT] Loaded. Restoring scene: ",
                          checkpoint_scene);
  return checkpoint_scene;
}

void MysteryManager::log_change(const String &p_type, const String &p_name,
                                const Variant &p_old, const Variant &p_new,
                                const String &p_caller) {
  UtilityFunctions::print("[LOG] ", p_name, ": ", p_old, " -> ", p_new,
                          " (Reason: ", p_caller, ")");
}

void MysteryManager::validate_state() {
  // Logical Contradiction Guardrails

  // 1. Solution flag cannot be true if evidence is missing
  if (get_flag("case_solved") && !get_flag("has_evidence_all")) {
    UtilityFunctions::print("[GUARDRAIL] CRITICAL: case_solved is true but "
                            "evidence is incomplete!");
    // In a real debug build, we might want to crash here or signal a major
    // error
  }

  // 2. Confrontation started without talking to everyone
  if (get_flag("confrontation_started") && !get_flag("talked_to_everyone")) {
    UtilityFunctions::print("[GUARDRAIL] WARNING: Confrontation started but "
                            "some NPCs might have been skipped.");
  }

  // 3. Health check
  auto *ags = karakuri::AdventureGameStateBase::get_singleton();
  if (ags && ags->get_health() <= 0 && !get_flag("game_over")) {
    UtilityFunctions::print(
        "[GUARDRAIL] CRITICAL: Health is 0 but game_over flag is not set!");
  }
}

} // namespace mystery
