#include "load_state_task.h"

#include "../mystery_manager.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void LoadStateTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_demo_id"), &LoadStateTask::get_demo_id);
}

karakuri::TaskResult LoadStateTask::execute(double /*delta*/) {
  if (!done_) {
    MysteryManager *mgr = MysteryManager::get_singleton();
    if (mgr == nullptr) {
      UtilityFunctions::push_error(
          "[LoadStateTask] MysteryManager が null です。");
      return karakuri::TaskResult::Failed;
    }
    bool ok = mgr->load_state(demo_id_);
    if (!ok) {
      UtilityFunctions::push_error(
          String("[LoadStateTask] ロードに失敗しました（セーブデータなし？）: ") +
          demo_id_);
    } else {
      UtilityFunctions::print("[Mystery] ロード完了: ", demo_id_);
    }
    done_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error LoadStateTask::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("value") && spec["value"].get_type() == Variant::STRING) {
    demo_id_ = spec["value"];
  }
  // demo_id_ のデフォルトは "mystery" (コンストラクタで設定済み)
  return godot::OK;
}

void LoadStateTask::complete_instantly() {
  execute(0.0);
}

} // namespace mystery
