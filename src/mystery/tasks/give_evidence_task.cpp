#include "give_evidence_task.h"

#include "../evidence_manager.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void GiveEvidenceTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_evidence_id"),
                       &GiveEvidenceTask::get_evidence_id);
}

karakuri::TaskResult GiveEvidenceTask::execute(double /*delta*/) {
  if (!done_) {
    if (evidence_id_.is_empty()) {
      UtilityFunctions::push_warning(
          "[GiveEvidenceTask] evidence_id が空です。スキップします。");
      done_ = true;
      return karakuri::TaskResult::Success;
    }
    EvidenceManager *em = EvidenceManager::get_singleton();
    if (em == nullptr) {
      UtilityFunctions::push_error(
          "[GiveEvidenceTask] EvidenceManager が null です。");
      return karakuri::TaskResult::Failed;
    }
    em->add_evidence(evidence_id_);
    UtilityFunctions::print("[Mystery] 証拠品を獲得: ", evidence_id_);
    done_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error GiveEvidenceTask::validate_and_setup(
    const godot::Dictionary &spec) {
  if (spec.has("value") && spec["value"].get_type() == Variant::STRING) {
    evidence_id_ = spec["value"];
  } else if (spec.has("evidence_id")) {
    evidence_id_ = spec["evidence_id"];
  } else {
    UtilityFunctions::push_error(
        "[GiveEvidenceTask] 必須キー 'value' (または 'evidence_id') "
        "が見つかりません。");
    return godot::ERR_INVALID_DATA;
  }
  return godot::OK;
}

void GiveEvidenceTask::complete_instantly() {
  execute(0.0);
}

} // namespace mystery
