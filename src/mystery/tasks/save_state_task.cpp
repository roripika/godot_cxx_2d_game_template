#include "save_state_task.h"

#include "../mystery_manager.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void SaveStateTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_demo_id"), &SaveStateTask::get_demo_id);
}

karakuri::TaskResult SaveStateTask::execute(double /*delta*/) {
  if (!done_) {
    MysteryManager *mgr = MysteryManager::get_singleton();
    if (mgr == nullptr) {
      UtilityFunctions::push_error(
          "[SaveStateTask] MysteryManager が null です。");
      return karakuri::TaskResult::Failed;
    }
    bool ok = mgr->save_state(demo_id_);
    if (!ok) {
      UtilityFunctions::push_error(
          String("[SaveStateTask] セーブに失敗しました: ") + demo_id_);
    } else {
      UtilityFunctions::print("[Mystery] セーブ完了: ", demo_id_);
    }
    done_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error SaveStateTask::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("value") && spec["value"].get_type() == Variant::STRING) {
    demo_id_ = spec["value"];
  }
  // demo_id_ のデフォルトは "mystery" (コンストラクタで設定済み)
  return godot::OK;
}

void SaveStateTask::complete_instantly() {
  execute(0.0);
}

} // namespace mystery
