#include "mystery_effect_map.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

MysteryEffectMap *MysteryEffectMap::singleton_ = nullptr;

MysteryEffectMap::MysteryEffectMap() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
}

MysteryEffectMap::~MysteryEffectMap() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

MysteryEffectMap *MysteryEffectMap::get_singleton() { return singleton_; }

void MysteryEffectMap::_bind_methods() {
  ClassDB::bind_method(D_METHOD("fire", "preset_name"),
                       &MysteryEffectMap::fire);
  ClassDB::bind_method(D_METHOD("register_preset", "name", "config"),
                       &MysteryEffectMap::register_preset);
  ClassDB::bind_method(D_METHOD("get_preset", "name"),
                       &MysteryEffectMap::get_preset);
  ClassDB::bind_method(D_METHOD("has_preset", "name"),
                       &MysteryEffectMap::has_preset);

  ClassDB::bind_method(D_METHOD("set_action_runner_path", "path"),
                       &MysteryEffectMap::set_action_runner_path);
  ClassDB::bind_method(D_METHOD("get_action_runner_path"),
                       &MysteryEffectMap::get_action_runner_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "action_runner_path",
                             PROPERTY_HINT_NODE_PATH_VALID_TYPES, "ActionRunner"),
               "set_action_runner_path", "get_action_runner_path");

  ClassDB::bind_method(D_METHOD("set_sound_service_path", "path"),
                       &MysteryEffectMap::set_sound_service_path);
  ClassDB::bind_method(D_METHOD("get_sound_service_path"),
                       &MysteryEffectMap::get_sound_service_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "sound_service_path",
                             PROPERTY_HINT_NODE_PATH_VALID_TYPES, "SoundService"),
               "set_sound_service_path", "get_sound_service_path");
}

// ---------------------------------------------------------------------------
// _ready — デフォルトプリセット登録
// ---------------------------------------------------------------------------
void MysteryEffectMap::_ready() {
  register_default_presets();
}

void MysteryEffectMap::register_default_presets() {
  // ----- item_collected (証拠品入手) -----
  {
    // action_steps: [{ type: zoom, params: {value:1.2, duration:0.15} }]
    Dictionary zoom_params;
    zoom_params["value"] = 1.2f;
    zoom_params["duration"] = 0.15f;
    zoom_params["back_duration"] = 0.2f;

    Dictionary zoom_step;
    zoom_step["type"] = "zoom";
    zoom_step["params"] = zoom_params;

    Array steps;
    steps.append(zoom_step);

    Dictionary cfg;
    cfg["action_steps"] = steps;
    cfg["se_path"] = String(""); // プロジェクト側で設定推奨
    cfg["vibrate_intensity"] = 0;
    cfg["vibrate_duration"] = 0.1f;
    presets_["item_collected"] = cfg;
  }

  // ----- contradict_impact (逆転・突きつけ成功) -----
  {
    Dictionary flash_params;
    flash_params["r"] = 1.0f;
    flash_params["g"] = 1.0f;
    flash_params["b"] = 1.0f;
    flash_params["alpha"] = 0.8f;
    flash_params["duration"] = 0.5f;

    Dictionary flash_step;
    flash_step["type"] = "flash";
    flash_step["params"] = flash_params;

    Dictionary shake_params;
    shake_params["intensity"] = 14.0f;
    shake_params["duration"] = 0.45f;
    shake_params["steps"] = 8;

    Dictionary shake_step;
    shake_step["type"] = "shake";
    shake_step["params"] = shake_params;

    Array steps;
    steps.append(flash_step);
    steps.append(shake_step);

    Dictionary cfg;
    cfg["action_steps"] = steps;
    cfg["se_path"] = String(""); // 例: "res://audio/se_objection.ogg"
    cfg["vibrate_intensity"] = 2;
    cfg["vibrate_duration"] = 0.4f;
    presets_["contradict_impact"] = cfg;
  }

  // ----- damage (ダメージ) -----
  {
    Dictionary shake_params;
    shake_params["intensity"] = 10.0f;
    shake_params["duration"] = 0.3f;
    shake_params["steps"] = 6;

    Dictionary shake_step;
    shake_step["type"] = "shake";
    shake_step["params"] = shake_params;

    Dictionary flash_params;
    flash_params["r"] = 1.0f;
    flash_params["g"] = 0.0f;
    flash_params["b"] = 0.0f;
    flash_params["alpha"] = 0.4f;
    flash_params["duration"] = 0.4f;

    Dictionary flash_step;
    flash_step["type"] = "flash";
    flash_step["params"] = flash_params;

    Array steps;
    steps.append(flash_step);
    steps.append(shake_step);

    Dictionary cfg;
    cfg["action_steps"] = steps;
    cfg["se_path"] = String(""); // 例: "res://audio/se_damage.ogg"
    cfg["vibrate_intensity"] = 1;
    cfg["vibrate_duration"] = 0.2f;
    presets_["damage"] = cfg;
  }
}

// ---------------------------------------------------------------------------
// ノード解決
// ---------------------------------------------------------------------------
karakuri::ActionRunner *MysteryEffectMap::get_action_runner() const {
  if (!action_runner_path_.is_empty()) {
    Node *n = get_node_or_null(action_runner_path_);
    if (n)
      return Object::cast_to<karakuri::ActionRunner>(n);
  }
  // パス未設定なら SoundService に近い場所から上位ツリーを検索しない
  // (循環防止のためパス設定を必須とする)
  return nullptr;
}

karakuri::SoundService *MysteryEffectMap::get_sound_service() const {
  // シングルトンを優先
  auto *svc = karakuri::SoundService::get_singleton();
  if (svc)
    return svc;
  // パス設定で解決
  if (!sound_service_path_.is_empty()) {
    Node *n = get_node_or_null(sound_service_path_);
    return Object::cast_to<karakuri::SoundService>(n);
  }
  return nullptr;
}

// ---------------------------------------------------------------------------
// 演出発火
// ---------------------------------------------------------------------------
void MysteryEffectMap::fire(const String &preset_name) {
  if (!presets_.has(preset_name)) {
    UtilityFunctions::push_warning(
        "MysteryEffectMap: unknown preset: " + preset_name);
    return;
  }

  Dictionary cfg = presets_[preset_name];

  // 1. ActionRunner 演出シーケンス
  if (cfg.has("action_steps")) {
    Variant v = cfg["action_steps"];
    if (v.get_type() == Variant::ARRAY) {
      karakuri::ActionRunner *runner = get_action_runner();
      if (runner) {
        runner->play_sequence(Array(v));
      } else {
        UtilityFunctions::print(
            "[MysteryEffectMap] ActionRunner not found for preset: " + preset_name);
      }
    }
  }

  // 2. SE 再生
  if (cfg.has("se_path")) {
    String se = cfg["se_path"];
    if (!se.is_empty()) {
      karakuri::SoundService *snd = get_sound_service();
      if (snd)
        snd->play_se(se);
    }
  }

  // 3. BGM 切替え（オプション）
  if (cfg.has("bgm_path")) {
    String bgm = cfg["bgm_path"];
    if (!bgm.is_empty()) {
      float fade = cfg.has("bgm_fade") ? float(cfg["bgm_fade"]) : 1.0f;
      karakuri::SoundService *snd = get_sound_service();
      if (snd)
        snd->play_bgm(bgm, fade);
    }
  }

  // 4. ハプティクス
  int vib_intensity = cfg.has("vibrate_intensity") ? int(cfg["vibrate_intensity"]) : 0;
  float vib_duration = cfg.has("vibrate_duration") ? float(cfg["vibrate_duration"]) : 0.0f;
  if (vib_duration > 0.0f) {
    karakuri::SoundService *snd = get_sound_service();
    if (snd)
      snd->vibrate(vib_intensity, vib_duration);
  }

  UtilityFunctions::print("[MysteryEffectMap] fired: " + preset_name);
}

// ---------------------------------------------------------------------------
// プリセット管理
// ---------------------------------------------------------------------------
void MysteryEffectMap::register_preset(const String &name,
                                       const Dictionary &config) {
  presets_[name] = config;
}

Dictionary MysteryEffectMap::get_preset(const String &name) const {
  if (presets_.has(name))
    return presets_[name];
  return Dictionary();
}

bool MysteryEffectMap::has_preset(const String &name) const {
  return presets_.has(name);
}

// ---------------------------------------------------------------------------
// プロパティ
// ---------------------------------------------------------------------------
void MysteryEffectMap::set_action_runner_path(const NodePath &path) {
  action_runner_path_ = path;
}
NodePath MysteryEffectMap::get_action_runner_path() const {
  return action_runner_path_;
}

void MysteryEffectMap::set_sound_service_path(const NodePath &path) {
  sound_service_path_ = path;
}
NodePath MysteryEffectMap::get_sound_service_path() const {
  return sound_service_path_;
}

} // namespace mystery
