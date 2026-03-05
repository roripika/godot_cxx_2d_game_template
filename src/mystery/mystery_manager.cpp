#include "mystery_manager.h"
#include "../core/adventure_game_state.h"
#include "../core/scenario/scenario_runner.h"
#include "evidence_manager.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
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

void MysteryManager::set_mystery_flag(mystery::MysteryFlag p_flag,
                                      bool p_value) {
  set_flag(MysteryEnumValueConverter::get_flag_key(p_flag), p_value);
}

bool MysteryManager::get_mystery_flag(mystery::MysteryFlag p_flag) const {
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

void MysteryManager::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;
  register_scenario_actions();
}

void MysteryManager::register_scenario_actions() {
  Node *root = get_tree()->get_root();
  // Search for ScenarioRunner in the tree
  // In our template it might be named "ScenarioRunner" or "DetectiveRunner"
  karakuri::ScenarioRunner *runner = nullptr;

  // Try to find it by class first (Duck typing or search)
  // Since we have the header, we can use find_child
  Node *n = root->find_child("ScenarioRunner", true, false);
  if (!n)
    n = root->find_child("DetectiveRunner", true, false);

  runner = Object::cast_to<karakuri::ScenarioRunner>(n);

  if (!runner) {
    UtilityFunctions::print(
        "[MysteryManager] ScenarioRunner NOT found. Skipping registration.");
    return;
  }

  UtilityFunctions::print(
      "[MysteryManager] Registering mystery actions to ScenarioRunner.");

  // 1. testimony
  runner->register_action("testimony", [this, runner](const Variant &p) {
    Dictionary d = p;
    Array testimonies = d.get("testimonies", Array());
    if (testimonies.is_empty())
      return false;

    UtilityFunctions::print("[Mystery] Starting testimony flow.");

    // We'll use a simple state to track current line of testimony
    // Since this is a lambda, we need to capture or use MysteryManager state.
    // For the demo, let's just show the first line and then wait for evidence
    // or press.

    Dictionary line = testimonies[0];
    Node *dialogue_ui =
        runner->get_node_or_null(runner->get_dialogue_ui_path());
    if (dialogue_ui && dialogue_ui->has_method("show_message")) {
      dialogue_ui->call("show_message", line.get("speaker", "Witness"),
                        line.get("text_key", "..."));
      // In a real implementation, we would store 'testimonies' in
      // MysteryManager and handle 'press' or 'present' signals. For now, we'll
      // just allow it to "block" indefinitely or until a signal.
    }

    return true; // Blocking
  });

  // 2. give_evidence (scenario uses 'give_evidence', not 'add_evidence')
  runner->register_action("give_evidence", [this](const Variant &p) {
    String id = p;
    EvidenceManager *em = EvidenceManager::get_singleton();
    if (em)
      em->add_evidence(id);
    return false;
  });

  // 3. take_damage
  runner->register_action("take_damage", [this](const Variant &p) {
    int amount = (int)p;
    auto *ags = karakuri::AdventureGameStateBase::get_singleton();
    if (ags)
      ags->set_health(ags->get_health() - amount);
    return false; // Non-blocking
  });
}

} // namespace mystery
