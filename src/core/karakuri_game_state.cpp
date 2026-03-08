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

} // namespace karakuri
