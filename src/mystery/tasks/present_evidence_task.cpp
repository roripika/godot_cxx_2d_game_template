#include "present_evidence_task.h"

#include "../../core/scenario/scenario_runner.h"
#include "../../core/services/item_service.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

karakuri::TaskResult PresentEvidenceTask::execute() {
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
    const karakuri::TaskSpec &spec) {
  PresentEvidenceTaskSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (!payload.has("value") || payload["value"].get_type() != godot::Variant::DICTIONARY) {
    godot::UtilityFunctions::push_error(
        "[PresentEvidenceTask] 'value' キーが DICTIONARY 型で必要です。");
    return godot::ERR_INVALID_DATA;
  }

  const godot::Dictionary params = payload["value"];

  if (!params.has("item_id") || params["item_id"].get_type() != godot::Variant::STRING) {
    godot::UtilityFunctions::push_error(
        "[PresentEvidenceTask] 'value.item_id' キーが STRING 型で必要です。");
    return godot::ERR_INVALID_DATA;
  }
  ts.item_id = params["item_id"];

  if (params.has("on_correct") && params["on_correct"].get_type() == godot::Variant::ARRAY) {
    ts.on_correct = params["on_correct"];
  }
  if (params.has("on_wrong") && params["on_wrong"].get_type() == godot::Variant::ARRAY) {
    ts.on_wrong = params["on_wrong"];
  }

  if (ts.on_correct.is_empty() && ts.on_wrong.is_empty()) {
    godot::UtilityFunctions::push_error(
        "[PresentEvidenceTask] 'on_correct' か 'on_wrong' の少なくとも一方が"
        "必要です。");
    return godot::ERR_INVALID_DATA;
  }

  item_id_ = ts.item_id;
  on_correct_ = ts.on_correct;
  on_wrong_ = ts.on_wrong;
  return godot::OK;
}

void PresentEvidenceTask::complete_instantly() {
  execute();
}

} // namespace mystery
