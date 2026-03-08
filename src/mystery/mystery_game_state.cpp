#include "mystery_game_state.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

MysteryGameState *MysteryGameState::singleton_ = nullptr;

MysteryGameState::MysteryGameState() {
  // MysteryGameState が Autoload に配置されたとき、KarakuriGameState::singleton_ も
  // 自身のポインタで上書きする。is-a 関係を利用することで、KarakuriGameState::get_singleton()
  // でも MysteryGameState インスタンスを取得できるようになる。
  singleton_ = this;
  karakuri::KarakuriGameState::singleton_ = this;
}

MysteryGameState::~MysteryGameState() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
  if (karakuri::KarakuriGameState::singleton_ == this) {
    karakuri::KarakuriGameState::singleton_ = nullptr;
  }
}

MysteryGameState *MysteryGameState::get_singleton() {
  return singleton_;
}

void MysteryGameState::_bind_methods() {
  // HP 管理
  ClassDB::bind_method(D_METHOD("set_health", "hp"),
                       &MysteryGameState::set_health);
  ClassDB::bind_method(D_METHOD("get_health"), &MysteryGameState::get_health);
  ClassDB::bind_method(D_METHOD("take_damage"), &MysteryGameState::take_damage);
  ClassDB::bind_method(D_METHOD("heal", "amount"), &MysteryGameState::heal);

  ADD_PROPERTY(PropertyInfo(Variant::INT, "health"), "set_health", "get_health");
  ADD_SIGNAL(MethodInfo("health_changed", PropertyInfo(Variant::INT, "hp")));

  // リセット
  ClassDB::bind_method(D_METHOD("reset_game"), &MysteryGameState::reset_game);

  // シングルトン取得（GDScript 向け）
  ClassDB::bind_static_method("MysteryGameState",
                              D_METHOD("get_singleton"),
                              &MysteryGameState::get_singleton);
}

// ------------------------------------------------------------------
// HP 管理
// ------------------------------------------------------------------

void MysteryGameState::set_health(int hp) {
  health_ = hp;
  UtilityFunctions::print("[MysteryGameState] health = ", health_);
  emit_signal("health_changed", health_);
}

int MysteryGameState::get_health() const {
  return health_;
}

void MysteryGameState::take_damage() {
  if (health_ > 0) {
    health_--;
    UtilityFunctions::print("[MysteryGameState] take_damage. health = ", health_);
    emit_signal("health_changed", health_);
  }
}

void MysteryGameState::heal(int amount) {
  health_ += amount;
  UtilityFunctions::print("[MysteryGameState] heal(", amount, "). health = ", health_);
  emit_signal("health_changed", health_);
}

// ------------------------------------------------------------------
// リセット（オーバーライド）
// ------------------------------------------------------------------

void MysteryGameState::reset_game() {
  // ① 基盤の reset_hook_ を呼び出す（EvidenceManager クリア等）
  karakuri::KarakuriGameState::reset_game();

  // ② Mystery 固有のリセット: HP を初期値 3 に戻す
  health_ = 3;
  UtilityFunctions::print("[MysteryGameState] reset_game(). health = ", health_);
  emit_signal("health_changed", health_);
}

} // namespace mystery
