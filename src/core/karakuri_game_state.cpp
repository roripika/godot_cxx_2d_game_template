#include "karakuri_game_state.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

KarakuriGameState *KarakuriGameState::singleton_ = nullptr;

KarakuriGameState::KarakuriGameState() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
}

KarakuriGameState::~KarakuriGameState() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

KarakuriGameState *KarakuriGameState::get_singleton() {
  return singleton_;
}

void KarakuriGameState::_bind_methods() {
  // リセット
  ClassDB::bind_method(D_METHOD("reset_game"), &KarakuriGameState::reset_game);
  ClassDB::bind_method(D_METHOD("set_reset_hook", "hook"),
                       &KarakuriGameState::set_reset_hook);

  // 汎用フラグ
  ClassDB::bind_method(D_METHOD("set_flag", "key", "value"),
                       &KarakuriGameState::set_flag);
  ClassDB::bind_method(D_METHOD("get_flag", "key", "default_value"),
                       &KarakuriGameState::get_flag, DEFVAL(Variant()));
  ClassDB::bind_method(D_METHOD("has_flag", "key"),
                       &KarakuriGameState::has_flag);

  // シングルトン取得（GDScript 向け）
  ClassDB::bind_static_method("KarakuriGameState",
                              D_METHOD("get_singleton"),
                              &KarakuriGameState::get_singleton);
}

void KarakuriGameState::reset_game() {
  if (reset_hook_.is_valid()) {
    reset_hook_.call();
  }
  UtilityFunctions::print("[KarakuriGameState] reset_game() called.");
}

void KarakuriGameState::set_reset_hook(const godot::Callable &hook) {
  reset_hook_ = hook;
}

// ------------------------------------------------------------------
// 汎用フラグ
// ------------------------------------------------------------------

void KarakuriGameState::set_flag(const godot::String &key,
                                  const godot::Variant &value) {
  global_flags_[key] = value;
}

godot::Variant KarakuriGameState::get_flag(const godot::String &key,
                                            const godot::Variant &default_value) const {
  if (!global_flags_.has(key)) {
    return default_value;
  }
  return global_flags_[key];
}

bool KarakuriGameState::has_flag(const godot::String &key) const {
  return global_flags_.has(key);
}

} // namespace karakuri
