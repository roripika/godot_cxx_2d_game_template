#include "mystery_game_state.h"

#include "core/action_registry.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

MysteryGameState *MysteryGameState::singleton_ = nullptr;

MysteryGameState::MysteryGameState() {
  // MysteryGameState が Autoload
  // に配置されたとき、KarakuriGameState::singleton_ も
  // 自身のポインタで上書きする。is-a
  // 関係を利用することで、KarakuriGameState::get_singleton() でも
  // MysteryGameState インスタンスを取得できるようになる。
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

MysteryGameState *MysteryGameState::get_singleton() { return singleton_; }

void MysteryGameState::_bind_methods() {
  // _ready() は Godot virtual メソッドのため bind_method 登録は不要。
  // GDExtension では C++ で override するだけで自動的に呼ばれる。

  // HP 管理
  ClassDB::bind_method(D_METHOD("set_health", "hp"),
                       &MysteryGameState::set_health);
  ClassDB::bind_method(D_METHOD("get_health"), &MysteryGameState::get_health);
  ClassDB::bind_method(D_METHOD("take_damage"), &MysteryGameState::take_damage);
  ClassDB::bind_method(D_METHOD("heal", "amount"), &MysteryGameState::heal);
  ClassDB::bind_method(D_METHOD("reset_health"),
                       &MysteryGameState::reset_health);

  ADD_PROPERTY(PropertyInfo(Variant::INT, "health"), "set_health",
               "get_health");
  ADD_SIGNAL(MethodInfo("health_changed", PropertyInfo(Variant::INT, "hp")));

  // 証拠品管理
  ClassDB::bind_method(D_METHOD("add_evidence", "evidence_id"),
                       &MysteryGameState::add_evidence);
  ClassDB::bind_method(D_METHOD("has_evidence", "evidence_id"),
                       &MysteryGameState::has_evidence);
  ClassDB::bind_method(D_METHOD("get_collected_evidences"),
                       &MysteryGameState::get_collected_evidences);
  ADD_SIGNAL(MethodInfo("evidence_added",
                        PropertyInfo(Variant::STRING, "evidence_id")));

  // リセット
  ClassDB::bind_method(D_METHOD("reset_game"), &MysteryGameState::reset_game);

  // 演出要求
  ClassDB::bind_method(D_METHOD("request_portrait", "character_id", "emotion"),
                       &MysteryGameState::request_portrait);
  ClassDB::bind_method(D_METHOD("request_background", "background_id"),
                       &MysteryGameState::request_background);

  ADD_SIGNAL(MethodInfo("portrait_requested",
                        PropertyInfo(Variant::STRING, "character_id"),
                        PropertyInfo(Variant::STRING, "emotion")));
  ADD_SIGNAL(MethodInfo("background_requested",
                        PropertyInfo(Variant::STRING, "background_id")));

  // シングルトン取得（GDScript 向け）
  ClassDB::bind_static_method("MysteryGameState", D_METHOD("get_singleton"),
                              &MysteryGameState::get_singleton);
}

// ------------------------------------------------------------------
// アクション登録 (_ready)
// ------------------------------------------------------------------

void MysteryGameState::_ready() {
  if (godot::Engine::get_singleton()->is_editor_hint())
    return;

  karakuri::ActionRegistry *reg = karakuri::ActionRegistry::get_singleton();
  if (reg == nullptr) {
    UtilityFunctions::push_error(
        "[MysteryGameState] ActionRegistry が null です。"
        "ActionRegistry を MysteryGameState より前に Autoload "
        "登録してください。");
    return;
  }

  // Mystery 固有のアクションを登録する。
  // core 層は Mystery 層の型を知らない・インクルードしない。
  // 文字列のクラス名だけで登録し、ClassDBSingleton が動的生成する。
  reg->register_action("add_evidence", "TaskAddEvidence");
  reg->register_action("show_portrait", "TaskShowPortrait");
  reg->register_action("change_background", "TaskChangeBackground");

  UtilityFunctions::print(
      "[MysteryGameState] Mystery アクションを登録しました。");
}

// ------------------------------------------------------------------
// HP 管理
// ------------------------------------------------------------------

void MysteryGameState::set_health(int hp) {
  health_ = hp;
  UtilityFunctions::print("[MysteryGameState] health = ", health_);
  emit_signal("health_changed", health_);
}

int MysteryGameState::get_health() const { return health_; }

void MysteryGameState::take_damage() {
  if (health_ > 0) {
    health_--;
    UtilityFunctions::print("[MysteryGameState] take_damage. health = ",
                            health_);
    emit_signal("health_changed", health_);
  }
}

void MysteryGameState::heal(int amount) {
  health_ += amount;
  UtilityFunctions::print("[MysteryGameState] heal(", amount,
                          "). health = ", health_);
  emit_signal("health_changed", health_);
}

void MysteryGameState::reset_health() {
  health_ = 3;
  UtilityFunctions::print("[MysteryGameState] reset_health(). health = ",
                          health_);
  emit_signal("health_changed", health_);
}

// ------------------------------------------------------------------
// 証拠品管理
// ------------------------------------------------------------------

void MysteryGameState::add_evidence(const String &evidence_id) {
  if (has_evidence(evidence_id)) {
    UtilityFunctions::print("[MysteryGameState] add_evidence: 既に所持 ",
                            evidence_id);
    return;
  }
  collected_evidences_.append(evidence_id);
  UtilityFunctions::print("[MysteryGameState] evidence added: ", evidence_id);
  emit_signal("evidence_added", evidence_id);
}

bool MysteryGameState::has_evidence(const String &evidence_id) const {
  return collected_evidences_.has(evidence_id);
}

TypedArray<String> MysteryGameState::get_collected_evidences() const {
  return collected_evidences_;
}

// ------------------------------------------------------------------
// リセット（オーバーライド）
// ------------------------------------------------------------------

void MysteryGameState::reset_game() {
  // ① 基盤の reset_hook_ を呼び出す（EvidenceManager クリア等）
  karakuri::KarakuriGameState::reset_game();

  // ② Mystery 固有のリセット: HP を初期値 3 に戻す
  health_ = 3;
  UtilityFunctions::print("[MysteryGameState] reset_game(). health = ",
                          health_);
  emit_signal("health_changed", health_);

  // ③ 証拠品リストもクリア
  collected_evidences_.clear();
  UtilityFunctions::print("[MysteryGameState] collected_evidences cleared.");
}

// ------------------------------------------------------------------
// 演出要求
// ------------------------------------------------------------------

void MysteryGameState::request_portrait(const godot::String &character_id,
                                        const godot::String &emotion) {
  UtilityFunctions::print("[MysteryGameState] portrait_requested: ",
                          character_id, " (", emotion, ")");
  emit_signal("portrait_requested", character_id, emotion);
}

void MysteryGameState::request_background(const godot::String &background_id) {
  UtilityFunctions::print("[MysteryGameState] background_requested: ",
                          background_id);
  emit_signal("background_requested", background_id);
}

} // namespace mystery
