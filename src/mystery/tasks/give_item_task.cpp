#include "give_item_task.h"

#include "../../core/services/item_service.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void GiveItemTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_item_id"), &GiveItemTask::get_item_id);
}

karakuri::TaskResult GiveItemTask::execute(double /*delta*/) {
  if (!done_) {
    karakuri::ItemService *svc = karakuri::ItemService::get_singleton();
    if (svc == nullptr) {
      UtilityFunctions::push_error(
          "[GiveItemTask] ItemService が null です。");
      return karakuri::TaskResult::Failed;
    }
    svc->add_item(item_id_);
    UtilityFunctions::print("[Mystery] アイテム取得: ", item_id_);
    done_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error GiveItemTask::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("value") && spec["value"].get_type() == Variant::STRING) {
    item_id_ = spec["value"];
  } else if (spec.has("item_id")) {
    item_id_ = spec["item_id"];
  } else {
    UtilityFunctions::push_error(
        "[GiveItemTask] 必須キー 'value' (または 'item_id') "
        "が見つかりません。");
    return godot::ERR_INVALID_DATA;
  }
  if (item_id_.is_empty()) {
    UtilityFunctions::push_error("[GiveItemTask] item_id が空文字列です。");
    return godot::ERR_INVALID_DATA;
  }
  return godot::OK;
}

void GiveItemTask::complete_instantly() {
  execute(0.0);
}

} // namespace mystery
