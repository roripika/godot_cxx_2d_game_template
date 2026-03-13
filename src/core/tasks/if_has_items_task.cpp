#include "if_has_items_task.h"
#include "../scenario/scenario_runner.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void IfHasItemsTask::_bind_methods() {}

TaskResult IfHasItemsTask::execute(double /*delta*/) {
  if (runner_ == nullptr) return TaskResult::Failed;

  // アイテムの所持確認ロジック。
  // Karakuri Kernel では InventoryService 等を介するべきだが、
  // 現状は MysteryGameState が管理している可能性が高い。
  // ここでは "MysteryGameState" シングルトンを探すか、
  // 汎用的な ItemService を想定する。
  
  bool all_found = true;
  Object *mystery_state = Engine::get_singleton()->get_singleton("MysteryGameState");
  
  if (mystery_state) {
    for (int i = 0; i < required_items_.size(); i++) {
        String item_id = required_items_[i];
        if (!bool(mystery_state->call("has_evidence", item_id))) {
            all_found = false;
            break;
        }
    }
  } else {
    // MysteryGameState がない場合は失敗（または true とみなすか？）
    all_found = false;
  }

  const Array &branch = all_found ? then_branch_ : else_branch_;
  if (!branch.is_empty()) {
    runner_->inject_steps(branch);
  }

  return TaskResult::Success;
}

Error IfHasItemsTask::validate_and_setup(const Dictionary &spec) {
  if (spec.has("items")) {
    required_items_ = spec["items"];
    then_branch_ = spec.has("then") ? Array(spec["then"]) : Array();
    else_branch_ = spec.has("else") ? Array(spec["else"]) : Array();
  } else {
    UtilityFunctions::push_error("IfHasItemsTask: 'items' key is missing from spec.");
    return ERR_INVALID_DATA;
  }
  return OK;
}

void IfHasItemsTask::complete_instantly() {
  execute(0.0);
}

} // namespace karakuri
