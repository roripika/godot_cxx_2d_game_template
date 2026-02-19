#include "karakuri_save_service.h"

#include "../core/adventure_game_state.h"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

String KarakuriSaveService::build_save_path(const String &demo_id) {
  if (demo_id.is_empty()) {
    return "user://karakuri/save.json";
  }
  return "user://karakuri/" + demo_id + "/save.json";
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool KarakuriSaveService::save_game(const String &demo_id) { // static
  AdventureGameStateBase *state = AdventureGameStateBase::get_singleton();
  if (!state) {
    UtilityFunctions::printerr("[KarakuriSaveService] save_game: "
                               "AdventureGameStateBase singleton not found.");
    return false;
  }

  // --- Build JSON payload ---
  Dictionary payload;

  // flags: Dictionary<String, bool>
  // AdventureGameStateBase stores flags as a Dictionary internally,
  // but we must reconstruct it from the public API via GDScript-compatible
  // methods. Since flags_ is private we expose a helper through state.
  // For now we call get_flag for each tracked key — but that requires knowing
  // all keys. Instead we use a Dictionary property accessor if available,
  // otherwise we snapshot only the known public fields.
  //
  // Solution: AdventureGameStateBase exposes no "get all flags" API yet.
  // We add a minimal snapshot by serializing via Godot's JSON utility.
  // The cleanest approach is to add get_flags_snapshot() to the state class,
  // but to avoid header changes here we store what we can through the GDScript
  // Variant bridge (call method by string name).
  Variant flags_variant = state->call("get_flags_snapshot");
  if (flags_variant.get_type() == Variant::DICTIONARY) {
    payload["flags"] = flags_variant;
  } else {
    payload["flags"] = Dictionary{};
  }

  Variant inv_variant = state->call("get_inventory_snapshot");
  if (inv_variant.get_type() == Variant::ARRAY) {
    payload["inventory"] = inv_variant;
  } else {
    payload["inventory"] = Array{};
  }

  payload["health"] = state->get_health();

  // --- Serialize ---
  String json_text = JSON::stringify(payload, "  ");

  // --- Ensure directory exists ---
  String path = build_save_path(demo_id);
  String dir;
  {
    int last_slash = path.rfind("/");
    dir = (last_slash >= 0) ? path.substr(0, last_slash) : "";
  }
  if (!dir.is_empty()) {
    DirAccess::make_dir_recursive_absolute(dir);
  }

  // --- Write ---
  Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE);
  if (file.is_null()) {
    UtilityFunctions::printerr("[KarakuriSaveService] save_game: "
                               "Failed to open for write: ",
                               path);
    return false;
  }
  file->store_string(json_text);
  UtilityFunctions::print("[KarakuriSaveService] Saved to: ", path);
  return true;
}

bool KarakuriSaveService::load_game(const String &demo_id) { // static
  AdventureGameStateBase *state = AdventureGameStateBase::get_singleton();
  if (!state) {
    UtilityFunctions::printerr("[KarakuriSaveService] load_game: "
                               "AdventureGameStateBase singleton not found.");
    return false;
  }

  String path = build_save_path(demo_id);
  if (!FileAccess::file_exists(path)) {
    UtilityFunctions::print("[KarakuriSaveService] load_game: "
                            "No save file at: ",
                            path);
    return false;
  }

  Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
  if (file.is_null()) {
    UtilityFunctions::printerr("[KarakuriSaveService] load_game: "
                               "Failed to open for read: ",
                               path);
    return false;
  }

  String json_text = file->get_as_text();

  // --- Parse ---
  Ref<JSON> parser;
  parser.instantiate();
  Error err = parser->parse(json_text);
  if (err != OK) {
    UtilityFunctions::printerr("[KarakuriSaveService] load_game: "
                               "JSON parse error at line ",
                               parser->get_error_line(), ": ",
                               parser->get_error_message());
    return false;
  }

  Variant result = parser->get_data();
  if (result.get_type() != Variant::DICTIONARY) {
    UtilityFunctions::printerr("[KarakuriSaveService] load_game: "
                               "Unexpected JSON root type.");
    return false;
  }

  Dictionary payload = result;

  // --- Restore flags ---
  if (payload.has("flags")) {
    Variant flags_v = payload["flags"];
    if (flags_v.get_type() == Variant::DICTIONARY) {
      state->call("restore_flags_snapshot", flags_v);
    }
  }

  // --- Restore inventory ---
  if (payload.has("inventory")) {
    Variant inv_v = payload["inventory"];
    if (inv_v.get_type() == Variant::ARRAY) {
      state->call("restore_inventory_snapshot", inv_v);
    }
  }

  // --- Restore health ---
  if (payload.has("health")) {
    Variant hp_v = payload["health"];
    if (hp_v.get_type() == Variant::INT ||
        hp_v.get_type() == Variant::FLOAT) {
      state->set_health(static_cast<int>(hp_v));
    }
  }

  UtilityFunctions::print("[KarakuriSaveService] Loaded from: ", path);
  return true;
}

bool KarakuriSaveService::has_save(const String &demo_id) { // static
  return FileAccess::file_exists(build_save_path(demo_id));
}

bool KarakuriSaveService::delete_save(const String &demo_id) { // static
  String path = build_save_path(demo_id);
  if (!FileAccess::file_exists(path)) {
    return false;
  }
  Error err = DirAccess::remove_absolute(path);
  return err == OK;
}

// ---------------------------------------------------------------------------
// Godot bindings
// ---------------------------------------------------------------------------

void KarakuriSaveService::_bind_methods() {
  ClassDB::bind_static_method(
      "KarakuriSaveService",
      D_METHOD("build_save_path", "demo_id"),
      &KarakuriSaveService::build_save_path);

  ClassDB::bind_static_method(
      "KarakuriSaveService",
      D_METHOD("save_game", "demo_id"),
      &KarakuriSaveService::save_game);
  ClassDB::bind_static_method(
      "KarakuriSaveService",
      D_METHOD("load_game", "demo_id"),
      &KarakuriSaveService::load_game);
  ClassDB::bind_static_method(
      "KarakuriSaveService",
      D_METHOD("has_save", "demo_id"),
      &KarakuriSaveService::has_save);
  ClassDB::bind_static_method(
      "KarakuriSaveService",
      D_METHOD("delete_save", "demo_id"),
      &KarakuriSaveService::delete_save);
}

} // namespace karakuri
