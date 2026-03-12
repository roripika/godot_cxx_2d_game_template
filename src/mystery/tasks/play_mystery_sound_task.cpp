#include "play_mystery_sound_task.h"

#include "../../core/services/sound_service.h"
#include "../mystery_effect_map.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void PlayMysterySoundTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_effect_map_path", "path"),
                       &PlayMysterySoundTask::set_effect_map_path);
  ClassDB::bind_method(D_METHOD("get_effect_map_path"),
                       &PlayMysterySoundTask::get_effect_map_path);

  ClassDB::bind_method(D_METHOD("set_preset_name", "name"),
                       &PlayMysterySoundTask::set_preset_name);
  ClassDB::bind_method(D_METHOD("get_preset_name"),
                       &PlayMysterySoundTask::get_preset_name);

  ClassDB::bind_method(D_METHOD("set_se_path", "path"),
                       &PlayMysterySoundTask::set_se_path);
  ClassDB::bind_method(D_METHOD("get_se_path"),
                       &PlayMysterySoundTask::get_se_path);

  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "effect_map_path"),
               "set_effect_map_path", "get_effect_map_path");
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "preset_name"),
               "set_preset_name", "get_preset_name");
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "se_path"),
               "set_se_path", "get_se_path");
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

karakuri::TaskResult PlayMysterySoundTask::execute(double /*delta*/) {
  if (!started_) {
    play_sound();
    started_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error PlayMysterySoundTask::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("preset_name")) {
    preset_name_ = spec["preset_name"];
  } else if (spec.has("preset")) {
    preset_name_ = spec["preset"];
  } else if (spec.has("se_path")) {
    se_path_ = spec["se_path"];
  } else {
    // どちらも空の場合は何もしないタスクとして許容するか、エラーにする。
    // ここでは ERR_INVALID_DATA を返す。
    return godot::ERR_INVALID_DATA;
  }

  if (spec.has("effect_map_path")) {
    effect_map_path_ = spec["effect_map_path"];
  }

  return godot::OK;
}

void PlayMysterySoundTask::complete_instantly() {
  execute(0.0);
}

// ------------------------------------------------------------------
// プロパティ
// ------------------------------------------------------------------

void PlayMysterySoundTask::set_effect_map_path(const NodePath &path) {
  effect_map_path_ = path;
}
NodePath PlayMysterySoundTask::get_effect_map_path() const {
  return effect_map_path_;
}

void PlayMysterySoundTask::set_preset_name(const String &name) {
  preset_name_ = name;
}
String PlayMysterySoundTask::get_preset_name() const { return preset_name_; }

void PlayMysterySoundTask::set_se_path(const String &path) { se_path_ = path; }
String PlayMysterySoundTask::get_se_path() const { return se_path_; }

} // namespace mystery
