#include "present_evidence_task.h"

#include "../../core/scenario/scenario_runner.h"
#include "../../core/services/item_service.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

karakuri::TaskResult PresentEvidenceTask::execute(double /*delta*/) {
  if (!done_) {
    if (runner_ == nullptr) {
      UtilityFunctions::push_error(
          "[PresentEvidenceTask] runner_ が null です。"
          "ScenarioRunner から set_runner() が呼ばれていません。");
      return karakuri::TaskResult::Failed;
    }

    karakuri::ItemService *svc = karakuri::ItemService::get_singleton();
    bool correct = svc && !item_id_.is_empty() && svc->has_item(item_id_);

    const godot::Array &branch = correct ? on_correct_ : on_wrong_;
    if (!branch.is_empty()) {
      runner_->inject_steps(branch);
    }

    UtilityFunctions::print("[Mystery] present_evidence '", item_id_,
                            "' -> ", correct ? "正解" : "不正解");
    done_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error PresentEvidenceTask::validate_and_setup(
    const godot::Dictionary &spec) {
  if (!spec.has("value") || spec["value"].get_type() != Variant::DICTIONARY) {
    UtilityFunctions::push_error(
        "[PresentEvidenceTask] 'value' キーが DICTIONARY 型で必要です。");
    return godot::ERR_INVALID_DATA;
  }

  const Dictionary params = spec["value"];

  if (!params.has("item_id") || params["item_id"].get_type() != Variant::STRING) {
    UtilityFunctions::push_error(
        "[PresentEvidenceTask] 'value.item_id' キーが STRING 型で必要です。");
    return godot::ERR_INVALID_DATA;
  }
  item_id_ = params["item_id"];

  if (params.has("on_correct") && params["on_correct"].get_type() == Variant::ARRAY) {
    on_correct_ = params["on_correct"];
  }
  if (params.has("on_wrong") && params["on_wrong"].get_type() == Variant::ARRAY) {
    on_wrong_ = params["on_wrong"];
  }

  if (on_correct_.is_empty() && on_wrong_.is_empty()) {
    UtilityFunctions::push_error(
        "[PresentEvidenceTask] 'on_correct' か 'on_wrong' の少なくとも一方が"
        "必要です。");
    return godot::ERR_INVALID_DATA;
  }

  return godot::OK;
}

void PresentEvidenceTask::complete_instantly() {
  execute(0.0);
}

} // namespace mystery
