#include "services/flag_service.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

FlagService *FlagService::singleton_ = nullptr;

FlagService::FlagService() {
  if (singleton_ == nullptr)
    singleton_ = this;
}

FlagService::~FlagService() {
  if (singleton_ == this)
    singleton_ = nullptr;
}

FlagService *FlagService::get_singleton() { return singleton_; }

void FlagService::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_flag", "name", "value"),
                       &FlagService::set_flag);
  ClassDB::bind_method(D_METHOD("get_flag", "name", "default"),
                       &FlagService::get_flag);
  ClassDB::bind_method(D_METHOD("has_flag", "name"), &FlagService::has_flag);
  ClassDB::bind_method(D_METHOD("erase_flag", "name"), &FlagService::erase_flag);
  ClassDB::bind_method(D_METHOD("clear_flags"), &FlagService::clear_flags);
  ClassDB::bind_method(D_METHOD("get_flag_names"), &FlagService::get_flag_names);
  ClassDB::bind_method(D_METHOD("sync_from", "data"), &FlagService::sync_from);
  ClassDB::bind_method(D_METHOD("serialize"), &FlagService::serialize);
  ClassDB::bind_method(D_METHOD("deserialize", "data"), &FlagService::deserialize);

  ADD_SIGNAL(MethodInfo("flag_changed",
                        PropertyInfo(Variant::STRING, "name"),
                        PropertyInfo(Variant::NIL, "value")));
}

// ---------------------------------------------------------------------------
// フラグ操作
// ---------------------------------------------------------------------------
void FlagService::set_flag(const String &name, const Variant &value) {
  flags_[name] = value;
  emit_signal("flag_changed", name, value);
}

Variant FlagService::get_flag(const String &name,
                              const Variant &p_default) const {
  if (flags_.has(name))
    return flags_[name];
  return p_default;
}

bool FlagService::has_flag(const String &name) const {
  return flags_.has(name);
}

void FlagService::erase_flag(const String &name) {
  flags_.erase(name);
}

void FlagService::clear_flags() {
  flags_.clear();
}

Array FlagService::get_flag_names() const {
  return flags_.keys();
}

void FlagService::sync_from(const Dictionary &data) {
  Array keys = data.keys();
  for (int i = 0; i < keys.size(); ++i) {
    String k = keys[i];
    flags_[k] = data[k];
  }
}

// ---------------------------------------------------------------------------
// シリアライズ
// ---------------------------------------------------------------------------
Dictionary FlagService::serialize() const {
  Dictionary d;
  d["flags"] = flags_.duplicate();
  return d;
}

void FlagService::deserialize(const Dictionary &data) {
  if (data.has("flags")) {
    Variant v = data["flags"];
    if (v.get_type() == Variant::DICTIONARY) {
      flags_ = Dictionary(v).duplicate();
    }
  }
}

} // namespace karakuri
