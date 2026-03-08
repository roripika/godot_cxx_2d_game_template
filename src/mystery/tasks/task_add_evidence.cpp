#include "task_add_evidence.h"

#include "../mystery_game_state.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void TaskAddEvidence::_bind_methods() {
  // evidence_id プロパティ
  ClassDB::bind_method(D_METHOD("set_evidence_id", "id"),
                       &TaskAddEvidence::set_evidence_id);
  ClassDB::bind_method(D_METHOD("get_evidence_id"),
                       &TaskAddEvidence::get_evidence_id);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "evidence_id"),
               "set_evidence_id", "get_evidence_id");
}

// ------------------------------------------------------------------
// ライフサイクル
// ------------------------------------------------------------------

void TaskAddEvidence::on_start() {
  if (evidence_id_.is_empty()) {
    UtilityFunctions::push_warning(
        "[TaskAddEvidence] evidence_id が空です。証拠品は追加されません。");
    finished_ = true;
    return;
  }

  MysteryGameState *mgs = MysteryGameState::get_singleton();
  if (mgs == nullptr) {
    UtilityFunctions::push_error(
        "[TaskAddEvidence] MysteryGameState が null です。"
        "Autoload として登録されているか確認してください。");
    finished_ = true;
    return;
  }

  mgs->add_evidence(evidence_id_);
  UtilityFunctions::print("[Mystery] 証拠品を獲得: ", evidence_id_);
  finished_ = true;
}

void TaskAddEvidence::complete_instantly() {
  // スキップ時も on_start() と同じ副作用を確実に適用する
  on_start();
}

// ------------------------------------------------------------------
// プロパティ
// ------------------------------------------------------------------

void TaskAddEvidence::set_evidence_id(const String &id) {
  evidence_id_ = id;
}

String TaskAddEvidence::get_evidence_id() const {
  return evidence_id_;
}

} // namespace mystery
