#include "karakuri_localization_service.h"

/**
 * @file karakuri_localization_service.cpp
 * @brief See karakuri_localization_service.h
 */

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void KarakuriLocalizationService::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_locale_store_path", "path"),
                       &KarakuriLocalizationService::set_locale_store_path);
  ClassDB::bind_method(D_METHOD("get_locale_store_path"),
                       &KarakuriLocalizationService::get_locale_store_path);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "locale_store_path"),
               "set_locale_store_path", "get_locale_store_path");

  ClassDB::bind_method(D_METHOD("set_locale_prefix", "prefix"),
                       &KarakuriLocalizationService::set_locale_prefix);
  ClassDB::bind_method(D_METHOD("save_current_locale_prefix"),
                       &KarakuriLocalizationService::save_current_locale_prefix);
  ClassDB::bind_method(D_METHOD("load_saved_locale_prefix"),
                       &KarakuriLocalizationService::load_saved_locale_prefix);
  ClassDB::bind_method(D_METHOD("get_current_locale_prefix"),
                       &KarakuriLocalizationService::get_current_locale_prefix);

  ADD_SIGNAL(
      MethodInfo("locale_changed", PropertyInfo(Variant::STRING, "locale")));
}

void KarakuriLocalizationService::_ready() { load_saved_locale_prefix(); }

void KarakuriLocalizationService::set_locale_store_path(const String &path) {
  if (path.is_empty()) {
    return;
  }
  locale_store_path_ = path;
}

String KarakuriLocalizationService::get_locale_store_path() const {
  return locale_store_path_;
}

String KarakuriLocalizationService::resolve_loaded_locale(
    const String &prefix) const {
  TranslationServer *ts = TranslationServer::get_singleton();
  if (!ts || prefix.is_empty()) {
    return prefix;
  }

  const PackedStringArray loaded = ts->get_loaded_locales();
  for (int i = 0; i < loaded.size(); i++) {
    const String locale = loaded[i];
    if (locale.begins_with(prefix)) {
      return locale;
    }
  }

  return prefix;
}

String KarakuriLocalizationService::extract_prefix(const String &locale) const {
  if (locale.is_empty()) {
    return "";
  }
  const int split = locale.find("_");
  if (split <= 0) {
    return locale;
  }
  return locale.substr(0, split);
}

void KarakuriLocalizationService::emit_locale_changed() {
  TranslationServer *ts = TranslationServer::get_singleton();
  if (!ts) {
    return;
  }
  emit_signal("locale_changed", ts->get_locale());
}

String KarakuriLocalizationService::set_locale_prefix(const String &prefix) {
  if (prefix.is_empty()) {
    return "";
  }
  TranslationServer *ts = TranslationServer::get_singleton();
  if (!ts) {
    return "";
  }

  const String resolved = resolve_loaded_locale(prefix);
  ts->set_locale(resolved);
  save_current_locale_prefix();
  emit_locale_changed();
  return resolved;
}

bool KarakuriLocalizationService::save_current_locale_prefix() {
  TranslationServer *ts = TranslationServer::get_singleton();
  if (!ts) {
    return false;
  }
  Ref<FileAccess> file = FileAccess::open(locale_store_path_, FileAccess::WRITE);
  if (file.is_null()) {
    // Directory may not exist yet; ensure it before retrying.
    DirAccess::make_dir_recursive_absolute("user://karakuri");
    file = FileAccess::open(locale_store_path_, FileAccess::WRITE);
  }
  if (file.is_null()) {
    UtilityFunctions::push_warning(
        "KarakuriLocalizationService: failed to open locale store path: ",
        locale_store_path_);
    return false;
  }
  file->store_line(extract_prefix(ts->get_locale()));
  return true;
}

String KarakuriLocalizationService::load_saved_locale_prefix() {
  TranslationServer *ts = TranslationServer::get_singleton();
  if (!ts) {
    return "";
  }

  Ref<FileAccess> file = FileAccess::open(locale_store_path_, FileAccess::READ);
  if (file.is_null()) {
    // First run: nothing saved yet. Keep current locale.
    emit_locale_changed();
    return ts->get_locale();
  }

  const String saved_prefix = file->get_as_text().strip_edges();
  if (saved_prefix.is_empty()) {
    emit_locale_changed();
    return ts->get_locale();
  }

  const String resolved = resolve_loaded_locale(saved_prefix);
  ts->set_locale(resolved);
  emit_locale_changed();
  return resolved;
}

String KarakuriLocalizationService::get_current_locale_prefix() const {
  TranslationServer *ts = TranslationServer::get_singleton();
  if (!ts) {
    return "";
  }
  return extract_prefix(ts->get_locale());
}

} // namespace karakuri
