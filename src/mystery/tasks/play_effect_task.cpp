#include "play_effect_task.h"

#include "../mystery_effect_map.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void PlayEffectTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_preset"), &PlayEffectTask::get_preset);
}

karakuri::TaskResult PlayEffectTask::execute() {
  if (!done_) {
    MysteryEffectMap *emap = MysteryEffectMap::get_singleton();
    if (emap == nullptr) {
      UtilityFunctions::push_error(
          "[PlayEffectTask] MysteryEffectMap が null です。");
      return karakuri::TaskResult::Failed;
    }
    emap->fire(preset_);
    UtilityFunctions::print("[Mystery] エフェクト発火: ", preset_);
    done_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error PlayEffectTask::validate_and_setup(
    const karakuri::TaskSpec &spec) {
  PlayEffectTaskSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("value") && payload["value"].get_type() == godot::Variant::STRING) {
    ts.preset = payload["value"];
  } else if (payload.has("preset") && payload["preset"].get_type() == godot::Variant::STRING) {
    ts.preset = payload["preset"];
  } else {
    godot::UtilityFunctions::push_error(
        "[PlayEffectTask] 必須キー 'value' (または 'preset'、STRING型) "
        "が見つかりません。");
    return godot::ERR_INVALID_DATA;
  }
  if (ts.preset.is_empty()) {
    godot::UtilityFunctions::push_error("[PlayEffectTask] preset が空文字列です。");
    return godot::ERR_INVALID_DATA;
  }

  preset_ = ts.preset;
  return godot::OK;
}

void PlayEffectTask::complete_instantly() {
  execute();
}

} // namespace mystery
