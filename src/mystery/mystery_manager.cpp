#include "mystery_manager.h"
#include "mystery_game_state.h"
#include "../core/scenario/scenario_runner.h"
#include "../core/services/item_service.h"
#include "../core/services/save_service.h"
#include "../core/world_state.h"
#include "evidence_manager.h"
#include "mystery_effect_map.h"
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
  ClassDB::bind_method(D_METHOD("save_state", "demo_id"),
                       &MysteryManager::save_state);
  ClassDB::bind_method(D_METHOD("load_state", "demo_id"),
                       &MysteryManager::load_state);
  ClassDB::bind_method(D_METHOD("has_save", "demo_id"),
                       &MysteryManager::has_save);
  ClassDB::bind_method(D_METHOD("delete_save", "demo_id"),
                       &MysteryManager::delete_save);
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
  bool old_value = get_flag(p_name);

  if (old_value != p_value) {
    auto *ws = karakuri::WorldState::get_singleton();
    if (ws) ws->set_state("mystery", karakuri::WorldState::SCOPE_GLOBAL, p_name, p_value);
    log_change("Flag", p_name, old_value, p_value, "set_flag");
    validate_state();
  }
}

bool MysteryManager::get_flag(const String &p_name) const {
  auto *ws = karakuri::WorldState::get_singleton();
  if (!ws) return false;
  return ws->has_flag("mystery", karakuri::WorldState::SCOPE_GLOBAL, p_name);
}

Dictionary MysteryManager::serialize_state() const {
  Dictionary dict;
  Dictionary flags_dict;
  auto *ws_ser = karakuri::WorldState::get_singleton();
  if (ws_ser) {
    // WorldState の mystery 名前空間・ SCOPE_GLOBAL のフラグをフラット辞書に展開
    Dictionary all_globals = ws_ser->serialize_globals();
    if (all_globals.has("global")) {
      Dictionary global_scope = Dictionary(all_globals["global"]);
      if (global_scope.has("mystery")) {
        flags_dict = Dictionary(global_scope["mystery"]).duplicate();
      }
    }
  }
  dict["flags"] = flags_dict;

  auto *em = EvidenceManager::get_singleton();
  if (em) {
    dict["evidence"] = em->serialize_evidence();
  }

  auto *mgs = MysteryGameState::get_singleton();
  if (mgs) {
    dict["health"] = mgs->get_health();
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
    auto *mgs = MysteryGameState::get_singleton();
    if (mgs) {
      mgs->set_health(p_dict["health"]);
    }
  }
}

void MysteryManager::deserialize_flags(const Dictionary &p_dict) {
  auto *ws = karakuri::WorldState::get_singleton();
  if (!ws) return;
  // 既存の mystery グローバルフラグをクリアしてから再ロード
  ws->clear_namespace("mystery");
  Array keys = p_dict.keys();
  for (int i = 0; i < keys.size(); i++) {
    String key = keys[i];
    ws->set_state("mystery", karakuri::WorldState::SCOPE_GLOBAL, key, p_dict[key]);
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
  auto *mgs = MysteryGameState::get_singleton();
  if (mgs && mgs->get_health() <= 0 && !get_flag("game_over")) {
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
    runner->emit_signal("dialogue_requested", line.get("speaker", "Witness"),
                        line.get("text_key", "..."));

    return true; // Blocking

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

  // 3. give_item — インベントリに追加 (give_evidence と独立した汎用版)
  runner->register_action("give_item", [](const Variant &p) {
    String id = p;
    auto *svc = karakuri::ItemService::get_singleton();
    if (svc)
      svc->add_item(id);
    return false; // Non-blocking
  });

  // 4. present_evidence — 証拠を提示してシナリオを分岐
  //    YAML: { action: present_evidence, value: { item_id: "...", on_correct: [...], on_wrong: [...] } }
  runner->register_action("present_evidence", [runner](const Variant &p) {
    if (p.get_type() != Variant::DICTIONARY) {
      return false;
    }
    Dictionary params = p;
    String item_id;
    if (params.has("item_id")) {
      item_id = String(params["item_id"]);
    }
    auto *svc = karakuri::ItemService::get_singleton();
    bool correct = svc && !item_id.is_empty() && svc->has_item(item_id);

    // on_correct / on_wrong いずれかのサブ配列でシナリオを差し込む
    String branch_key = correct ? "on_correct" : "on_wrong";
    if (params.has(branch_key)) {
      Variant branch = params[branch_key];
      if (branch.get_type() == Variant::ARRAY) {
        Array steps = branch;
        runner->inject_steps(steps);
      }
    }
    return false; // Non-blocking (inject した分は次フレームで実行)
  });

  // 5. play_effect — MysteryEffectMap のプリセット発火
  //    YAML: { action: play_effect, value: "contradict_impact" }
  runner->register_action("play_effect", [](const Variant &p) {
    String preset = p.get_type() == Variant::STRING ? (String)p : String("");
    if (preset.is_empty())
      return false;
    auto *emap = MysteryEffectMap::get_singleton();
    if (emap)
      emap->fire(preset);
    return false; // Non-blocking
  });

  // 6. take_damage
  runner->register_action("take_damage", [this](const Variant &p) {
    auto *mgs = MysteryGameState::get_singleton();
    if (mgs)
      mgs->take_damage();
    return false; // Non-blocking
  });

  // 4. save  — YAML: { action: save, value: "mystery" }
  runner->register_action("save", [this](const Variant &p) {
    String demo_id = p.get_type() == Variant::STRING ? (String)p : "mystery";
    save_state(demo_id);
    return false; // Non-blocking
  });

  // 5. load  — YAML: { action: load, value: "mystery" }
  runner->register_action("load", [this](const Variant &p) {
    String demo_id = p.get_type() == Variant::STRING ? (String)p : "mystery";
    load_state(demo_id);
    return false; // Non-blocking
  });
}

// ---------------------------------------------------------------------------
// Persistence: SaveService 経由でセーブ・ロード
// ---------------------------------------------------------------------------

bool MysteryManager::save_state(const String &demo_id) {
  // MysteryManager::serialize_state() が flags + evidence + health をまとめる
  Dictionary data = serialize_state();
  return karakuri::SaveService::save_game(demo_id, data);
}

bool MysteryManager::load_state(const String &demo_id) {
  Dictionary data = karakuri::SaveService::load_game(demo_id);
  if (data.is_empty()) {
    return false;
  }
  deserialize_state(data);
  return true;
}

bool MysteryManager::has_save(const String &demo_id) const {
  return karakuri::SaveService::has_save(demo_id);
}

bool MysteryManager::delete_save(const String &demo_id) {
  return karakuri::SaveService::delete_save(demo_id);
}

} // namespace mystery
