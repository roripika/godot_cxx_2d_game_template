#include "services/sound_service.h"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/callback_tweener.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/interval_tweener.hpp>
#include <godot_cpp/classes/property_tweener.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

SoundService *SoundService::singleton_ = nullptr;

SoundService::SoundService() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
}

SoundService::~SoundService() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

SoundService *SoundService::get_singleton() { return singleton_; }

void SoundService::_bind_methods() {
  ClassDB::bind_method(D_METHOD("play_bgm", "stream_path", "fade_sec"),
                       &SoundService::play_bgm);
  ClassDB::bind_method(D_METHOD("stop_bgm", "fade_sec"), &SoundService::stop_bgm);
  ClassDB::bind_method(D_METHOD("play_se", "stream_path", "volume_db"),
                       &SoundService::play_se);
  ClassDB::bind_method(D_METHOD("vibrate", "intensity", "duration_sec"),
                       &SoundService::vibrate);

  ClassDB::bind_method(D_METHOD("set_bgm_volume_db", "db"),
                       &SoundService::set_bgm_volume_db);
  ClassDB::bind_method(D_METHOD("get_bgm_volume_db"),
                       &SoundService::get_bgm_volume_db);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bgm_volume_db", PROPERTY_HINT_RANGE,
                             "-80,6,0.5"), "set_bgm_volume_db", "get_bgm_volume_db");

  ClassDB::bind_method(D_METHOD("set_se_volume_db", "db"),
                       &SoundService::set_se_volume_db);
  ClassDB::bind_method(D_METHOD("get_se_volume_db"),
                       &SoundService::get_se_volume_db);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "se_volume_db", PROPERTY_HINT_RANGE,
                             "-80,6,0.5"), "set_se_volume_db", "get_se_volume_db");
}

// ---------------------------------------------------------------------------
// _ready — プレイヤーノードを生成
// ---------------------------------------------------------------------------
void SoundService::_ready() {
  ensure_players();
}

void SoundService::ensure_players() {
  // BGM × 2 (クロスフェード用)
  for (int i = 0; i < 2; ++i) {
    if (!bgm_players_[i]) {
      auto *p = memnew(AudioStreamPlayer);
      p->set_name(String("BGM") + String::num(i));
      p->set_volume_db(-80.0f); // 初期状態はミュート
      add_child(p);
      bgm_players_[i] = p;
    }
  }
  // SE プール × 8
  for (int i = 0; i < SE_POOL_SIZE; ++i) {
    if (!se_pool_[i]) {
      auto *p = memnew(AudioStreamPlayer);
      p->set_name(String("SE") + String::num(i));
      p->set_volume_db(se_volume_db_);
      add_child(p);
      se_pool_[i] = p;
    }
  }
}

// ---------------------------------------------------------------------------
// BGM
// ---------------------------------------------------------------------------
void SoundService::play_bgm(const String &stream_path, float fade_sec) {
  if (stream_path == current_bgm_path_)
    return; // 同じトラックは再生済み

  ensure_players();

  // ストリームをロード
  Ref<Resource> res = ResourceLoader::get_singleton()->load(stream_path);
  Ref<AudioStream> stream = res;
  if (!stream.is_valid()) {
    UtilityFunctions::push_warning("SoundService: BGM not found: " + stream_path);
    return;
  }

  int next = 1 - bgm_active_;
  AudioStreamPlayer *next_player = Object::cast_to<AudioStreamPlayer>(bgm_players_[next]);
  AudioStreamPlayer *curr_player = Object::cast_to<AudioStreamPlayer>(bgm_players_[bgm_active_]);
  if (!next_player)
    return;

  next_player->set_stream(stream);
  next_player->set_volume_db(fade_sec > 0.0f ? -80.0f : bgm_volume_db_);
  next_player->play();

  if (fade_sec > 0.0f) {
    // クロスフェード: next は -80 → 基本音量、curr は 基本音量 → -80
    Ref<Tween> tween = get_tree()->create_tween();
    tween->set_parallel(true);
    tween->tween_property(next_player, "volume_db", bgm_volume_db_, fade_sec)
        ->set_trans(Tween::TRANS_LINEAR);
    if (curr_player && curr_player->is_playing()) {
      tween->tween_property(curr_player, "volume_db", -80.0f, fade_sec)
          ->set_trans(Tween::TRANS_LINEAR);
      // フェード後に停止
      Ref<Tween> stop_tween = get_tree()->create_tween();
      stop_tween->tween_interval(fade_sec);
      stop_tween->tween_callback(Callable(curr_player, "stop"));
    }
  } else {
    // 即時切替え
    if (curr_player && curr_player->is_playing())
      curr_player->stop();
    next_player->set_volume_db(bgm_volume_db_);
  }

  bgm_active_ = next;
  current_bgm_path_ = stream_path;
}

void SoundService::stop_bgm(float fade_sec) {
  ensure_players();
  AudioStreamPlayer *curr = Object::cast_to<AudioStreamPlayer>(bgm_players_[bgm_active_]);
  if (!curr || !curr->is_playing())
    return;

  if (fade_sec > 0.0f) {
    Ref<Tween> tween = get_tree()->create_tween();
    tween->tween_property(curr, "volume_db", -80.0f, fade_sec)
        ->set_trans(Tween::TRANS_LINEAR);
    tween->tween_callback(Callable(curr, "stop"));
  } else {
    curr->stop();
  }
  current_bgm_path_ = "";
}

// ---------------------------------------------------------------------------
// SE
// ---------------------------------------------------------------------------
Node *SoundService::get_free_se_player() {
  for (int i = 0; i < SE_POOL_SIZE; ++i) {
    AudioStreamPlayer *p = Object::cast_to<AudioStreamPlayer>(se_pool_[i]);
    if (p && !p->is_playing())
      return se_pool_[i];
  }
  return se_pool_[0]; // 全チャンネル使用中の場合は 0 番を上書き
}

void SoundService::play_se(const String &stream_path, float volume_db) {
  ensure_players();

  Ref<Resource> res = ResourceLoader::get_singleton()->load(stream_path);
  Ref<AudioStream> stream = res;
  if (!stream.is_valid()) {
    UtilityFunctions::push_warning("SoundService: SE not found: " + stream_path);
    return;
  }

  AudioStreamPlayer *p = Object::cast_to<AudioStreamPlayer>(get_free_se_player());
  if (!p)
    return;
  p->set_stream(stream);
  p->set_volume_db(se_volume_db_ + volume_db);
  p->play();
}

// ---------------------------------------------------------------------------
// ハプティクス
// ---------------------------------------------------------------------------
void SoundService::vibrate(int intensity, float duration_sec) {
  Input *input = Input::get_singleton();
  if (!input)
    return;
  // Godot 4 の vibrate_handheld(duration_ms)
  int duration_ms = static_cast<int>(duration_sec * 1000.0f);
  // intensity の分だけ強度を変える（Godot 4.4+ では amplitude 引数あり）
  input->vibrate_handheld(duration_ms, float(intensity) < 2.0f ? float(intensity) * 0.5f : 1.0f);
}

// ---------------------------------------------------------------------------
// 音量設定
// ---------------------------------------------------------------------------
void SoundService::set_bgm_volume_db(float db) { bgm_volume_db_ = db; }
float SoundService::get_bgm_volume_db() const { return bgm_volume_db_; }

void SoundService::set_se_volume_db(float db) { se_volume_db_ = db; }
float SoundService::get_se_volume_db() const { return se_volume_db_; }

} // namespace karakuri
