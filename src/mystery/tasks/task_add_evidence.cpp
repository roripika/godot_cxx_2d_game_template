#include "task_add_evidence.h"

#include "../mystery_game_state.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void TaskAddEvidence::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_evidence_id"),
                       &TaskAddEvidence::get_evidence_id);
}

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

karakuri::TaskResult TaskAddEvidence::execute(double /*delta*/) {
  if (!started_) {
    MysteryGameState *mgs = MysteryGameState::get_singleton();
    if (mgs == nullptr) {
      UtilityFunctions::push_error(
          "[TaskAddEvidence] MysteryGameState が null です。");
      return karakuri::TaskResult::Failed;
    }

    if (evidence_id_.is_empty()) {
      UtilityFunctions::push_warning(
          "[TaskAddEvidence] evidence_id が空です。");
    } else {
      mgs->add_evidence(evidence_id_);
      UtilityFunctions::print("[Mystery] 証拠品を獲得: ", evidence_id_);
    }
    started_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error TaskAddEvidence::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("value")) {
    evidence_id_ = spec["value"];
  } else if (spec.has("evidence_id")) {
    evidence_id_ = spec["evidence_id"];
  } else {
    // 証拠品追加タスクには最低限 ID が必要
    return godot::ERR_INVALID_DATA;
  }
  return godot::OK;
}

void TaskAddEvidence::complete_instantly() {
  execute(0.0);
}

// ------------------------------------------------------------------
// プロパティ
// ------------------------------------------------------------------

String TaskAddEvidence::get_evidence_id() const {
  return evidence_id_;
}

} // namespace mystery
