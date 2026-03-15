#include "play_mystery_sound_task.h"

#include "../../core/services/sound_service.h"
#include "../mystery_effect_map.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void PlayMysterySoundTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_effect_map_path"),
                       &PlayMysterySoundTask::get_effect_map_path);

  ClassDB::bind_method(D_METHOD("get_preset_name"),
                       &PlayMysterySoundTask::get_preset_name);

  ClassDB::bind_method(D_METHOD("get_se_path"),
                       &PlayMysterySoundTask::get_se_path);
}

// ------------------------------------------------------------------
// ライフサイクル
// ------------------------------------------------------------------

void PlayMysterySoundTask::play_sound() {
  if (!preset_name_.is_empty()) {
    // MysteryEffectMap シングルトンを直接取得して fire()
    auto *effect_map = mystery::MysteryEffectMap::get_singleton();
    if (effect_map != nullptr) {
      effect_map->fire(preset_name_);
    }
  } else if (!se_path_.is_empty()) {
    auto *ss = karakuri::SoundService::get_singleton();
    if (ss != nullptr) {
      ss->play_se(se_path_);
    }
  }
}

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

karakuri::TaskResult PlayMysterySoundTask::execute() {
  if (!started_) {
    play_sound();
    started_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error PlayMysterySoundTask::validate_and_setup(const karakuri::TaskSpec &spec) {
  PlayMysterySoundTaskSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("preset_name")) {
    ts.preset_name = payload["preset_name"];
  } else if (payload.has("preset")) {
    ts.preset_name = payload["preset"];
  } else if (payload.has("se_path")) {
    ts.se_path = payload["se_path"];
  } else {
    // どちらも空の場合は何もしないタスクとして許容するか、エラーにする。
    // ここでは ERR_INVALID_DATA を返す。
    return godot::ERR_INVALID_DATA;
  }

  if (payload.has("effect_map_path")) {
    ts.effect_map_path = payload["effect_map_path"];
  }

  preset_name_ = ts.preset_name;
  se_path_ = ts.se_path;
  effect_map_path_ = ts.effect_map_path;
  return godot::OK;
}

void PlayMysterySoundTask::complete_instantly() {
  execute();
}

// ------------------------------------------------------------------
// プロパティ
// ------------------------------------------------------------------

NodePath PlayMysterySoundTask::get_effect_map_path() const {
  return effect_map_path_;
}

String PlayMysterySoundTask::get_preset_name() const { return preset_name_; }

String PlayMysterySoundTask::get_se_path() const { return se_path_; }

} // namespace mystery
