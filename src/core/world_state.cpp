#include "world_state.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

WorldState *WorldState::singleton_ = nullptr;

// ---------------------------------------------------------------------------
// コンストラクタ / デストラクタ
// ---------------------------------------------------------------------------

WorldState::WorldState() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
}

WorldState::~WorldState() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

WorldState *WorldState::get_singleton() { return singleton_; }

// ---------------------------------------------------------------------------
// _bind_methods
// ---------------------------------------------------------------------------

void WorldState::_bind_methods() {
  // Enum バインド
  BIND_ENUM_CONSTANT(SCOPE_GLOBAL);
  BIND_ENUM_CONSTANT(SCOPE_SESSION);
  BIND_ENUM_CONSTANT(SCOPE_SCENE);

  // 主要 API
  ClassDB::bind_method(
      D_METHOD("set_state", "namespace", "scope", "key", "value"),
      &WorldState::set_state);
  ClassDB::bind_method(
      D_METHOD("get_state", "namespace", "scope", "key", "default_val"),
      &WorldState::get_state, DEFVAL(Variant()));
  ClassDB::bind_method(
      D_METHOD("has_flag", "namespace", "scope", "key"),
      &WorldState::has_flag);
  ClassDB::bind_method(D_METHOD("clear_scope", "scope"),
                       &WorldState::clear_scope);
  ClassDB::bind_method(D_METHOD("clear_namespace", "namespace"),
                       &WorldState::clear_namespace);
  ClassDB::bind_method(D_METHOD("serialize_globals"),
                       &WorldState::serialize_globals);
  ClassDB::bind_method(D_METHOD("deserialize_globals", "data"),
                       &WorldState::deserialize_globals);

  // 後方互換ラッパー
  ClassDB::bind_method(D_METHOD("set_flag", "key", "value"),
                       &WorldState::set_flag);
  ClassDB::bind_method(D_METHOD("get_flag", "key", "default_val"),
                       &WorldState::get_flag, DEFVAL(Variant()));
  ClassDB::bind_method(D_METHOD("has_flag_simple", "key"),
                       &WorldState::has_flag_simple);
  ClassDB::bind_method(D_METHOD("erase_flag", "key"),
                       &WorldState::erase_flag);

  // シグナル
  ADD_SIGNAL(MethodInfo("state_changed",
                        PropertyInfo(Variant::STRING,  "namespace"),
                        PropertyInfo(Variant::INT,     "scope"),
                        PropertyInfo(Variant::STRING,  "key"),
                        PropertyInfo(Variant::NIL,     "old_value"),
                        PropertyInfo(Variant::NIL,     "new_value")));
  ADD_SIGNAL(MethodInfo("scope_cleared",
                        PropertyInfo(Variant::INT,     "scope")));
}

// ---------------------------------------------------------------------------
// 内部ヘルパー: スコープ辞書の取得
// ---------------------------------------------------------------------------

Dictionary *WorldState::get_scope_dict(int scope) {
  switch (scope) {
    case SCOPE_SESSION: return &session_states_;
    case SCOPE_SCENE:   return &scene_states_;
    default:            return &global_states_;
  }
}

const Dictionary *WorldState::get_scope_dict(int scope) const {
  switch (scope) {
    case SCOPE_SESSION: return &session_states_;
    case SCOPE_SCENE:   return &scene_states_;
    default:            return &global_states_;
  }
}

// ---------------------------------------------------------------------------
// 主要 API 実装
// ---------------------------------------------------------------------------

void WorldState::set_state(const String &ns, int scope,
                           const String &key, const Variant &value) {
  Dictionary *d = get_scope_dict(scope);

  Variant old_value;
  Dictionary ns_dict;
  if (d->has(ns)) {
    Variant v = (*d)[ns];
    if (v.get_type() == Variant::DICTIONARY) {
      ns_dict = Dictionary(v);
      if (ns_dict.has(key)) {
        old_value = ns_dict[key];
      }
    }
  }
  ns_dict[key] = value;
  (*d)[ns] = ns_dict;

  emit_signal("state_changed", ns, scope, key, old_value, value);
}

Variant WorldState::get_state(const String &ns, int scope,
                              const String &key,
                              const Variant &default_val) const {
  const Dictionary *d = get_scope_dict(scope);
  if (!d->has(ns)) return default_val;

  Variant v = (*d)[ns];
  if (v.get_type() != Variant::DICTIONARY) return default_val;

  Dictionary ns_dict = Dictionary(v);
  if (!ns_dict.has(key)) return default_val;
  return ns_dict[key];
}

bool WorldState::has_flag(const String &ns, int scope,
                          const String &key) const {
  Variant v = get_state(ns, scope, key);
  if (v.get_type() == Variant::NIL)  return false;
  if (v.get_type() == Variant::BOOL) return bool(v);
  // 非NIL・非boolの値は「存在している」とみなして true を返す
  return true;
}

void WorldState::clear_scope(int scope) {
  Dictionary *d = get_scope_dict(scope);
  d->clear();
  emit_signal("scope_cleared", scope);
}

void WorldState::clear_namespace(const String &ns) {
  global_states_.erase(ns);
  session_states_.erase(ns);
  scene_states_.erase(ns);
}

// ---------------------------------------------------------------------------
// シリアライズ / デシリアライズ (SCOPE_GLOBAL のみ)
// ---------------------------------------------------------------------------

Dictionary WorldState::serialize_globals() const {
  Dictionary out;
  out["global"] = global_states_.duplicate(true);
  return out;
}

void WorldState::deserialize_globals(const Dictionary &data) {
  if (data.has("global")) {
    Variant v = data["global"];
    if (v.get_type() == Variant::DICTIONARY) {
      global_states_ = Dictionary(v).duplicate(true);
    }
  }
}

// ---------------------------------------------------------------------------
// 後方互換ラッパー (旧 FlagService API)
// namespace = "core", scope = SCOPE_GLOBAL 固定で委譲する。
// ---------------------------------------------------------------------------

static const char *kCompatNS = "core";

void WorldState::set_flag(const String &key, const Variant &value) {
  set_state(kCompatNS, SCOPE_GLOBAL, key, value);
}

Variant WorldState::get_flag(const String &key,
                             const Variant &default_val) const {
  return get_state(kCompatNS, SCOPE_GLOBAL, key, default_val);
}

bool WorldState::has_flag_simple(const String &key) const {
  return has_flag(kCompatNS, SCOPE_GLOBAL, key);
}

void WorldState::erase_flag(const String &key) {
  // namespace サブ辞書から key を削除する
  if (!global_states_.has(String(kCompatNS))) return;
  Variant v = global_states_[String(kCompatNS)];
  if (v.get_type() != Variant::DICTIONARY) return;
  Dictionary ns_dict = Dictionary(v);
  ns_dict.erase(key);
  global_states_[String(kCompatNS)] = ns_dict;
}

} // namespace karakuri
