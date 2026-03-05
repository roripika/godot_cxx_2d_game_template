#!/usr/bin/env python3
"""Write clean save_service.cpp (generic API, no AdventureGameStateBase dependency)."""
import os

WORKSPACE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(WORKSPACE, "src/core/services/save_service.cpp")

content = r"""#include "save_service.h"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

// ---------------------------------------------------------------------------
// Path helper
// ---------------------------------------------------------------------------

String SaveService::build_save_path(const String &filename) {
  return "user://karakuri/" + filename + ".json";
}

// ---------------------------------------------------------------------------
// save_game: Dictionary を JSON ファイルに書き出す
// ---------------------------------------------------------------------------

bool SaveService::save_game(const String &filename, const Dictionary &data) {
  String path = build_save_path(filename);

  // ディレクトリが存在しない場合は作成
  String dir;
  {
    int last_slash = path.rfind("/");
    dir = (last_slash >= 0) ? path.substr(0, last_slash) : "";
  }
  if (!dir.is_empty()) {
    DirAccess::make_dir_recursive_absolute(dir);
  }

  // JSON シリアライズ
  String json_text = JSON::stringify(data, "  ");

  // ファイル書き込み
  Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE);
  if (file.is_null()) {
    UtilityFunctions::printerr("[SaveService] save_game: "
                               "Failed to open for write: ",
                               path);
    return false;
  }
  file->store_string(json_text);
  UtilityFunctions::print("[SaveService] Saved: ", path);
  return true;
}

// ---------------------------------------------------------------------------
// load_game: JSON ファイルを読み込んで Dictionary を返す
// ---------------------------------------------------------------------------

Dictionary SaveService::load_game(const String &filename) {
  String path = build_save_path(filename);

  if (!FileAccess::file_exists(path)) {
    UtilityFunctions::print("[SaveService] load_game: no file at: ", path);
    return Dictionary{};
  }

  Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
  if (file.is_null()) {
    UtilityFunctions::printerr("[SaveService] load_game: "
                               "Failed to open for read: ",
                               path);
    return Dictionary{};
  }

  String json_text = file->get_as_text();

  Ref<JSON> parser;
  parser.instantiate();
  Error err = parser->parse(json_text);
  if (err != OK) {
    UtilityFunctions::printerr("[SaveService] load_game: "
                               "JSON parse error at line ",
                               parser->get_error_line(), ": ",
                               parser->get_error_message());
    return Dictionary{};
  }

  Variant result = parser->get_data();
  if (result.get_type() != Variant::DICTIONARY) {
    UtilityFunctions::printerr("[SaveService] load_game: "
                               "Unexpected JSON root type.");
    return Dictionary{};
  }

  UtilityFunctions::print("[SaveService] Loaded: ", path);
  return static_cast<Dictionary>(result);
}

// ---------------------------------------------------------------------------
// has_save / delete_save
// ---------------------------------------------------------------------------

bool SaveService::has_save(const String &filename) {
  return FileAccess::file_exists(build_save_path(filename));
}

bool SaveService::delete_save(const String &filename) {
  String path = build_save_path(filename);
  if (!FileAccess::file_exists(path)) {
    return false;
  }
  return DirAccess::remove_absolute(path) == OK;
}

// ---------------------------------------------------------------------------
// Godot bindings
// ---------------------------------------------------------------------------

void SaveService::_bind_methods() {
  ClassDB::bind_static_method(
      "SaveService",
      D_METHOD("build_save_path", "filename"),
      &SaveService::build_save_path);
  ClassDB::bind_static_method(
      "SaveService",
      D_METHOD("save_game", "filename", "data"),
      &SaveService::save_game);
  ClassDB::bind_static_method(
      "SaveService",
      D_METHOD("load_game", "filename"),
      &SaveService::load_game);
  ClassDB::bind_static_method(
      "SaveService",
      D_METHOD("has_save", "filename"),
      &SaveService::has_save);
  ClassDB::bind_static_method(
      "SaveService",
      D_METHOD("delete_save", "filename"),
      &SaveService::delete_save);
}

} // namespace karakuri
"""

with open(path, 'w') as f:
    f.write(content)
print(f"Written: {path}")
print(f"Lines: {len(content.splitlines())}")
