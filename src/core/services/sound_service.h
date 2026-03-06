#ifndef KARAKURI_SOUND_SERVICE_H
#define KARAKURI_SOUND_SERVICE_H

/**
 * @file sound_service.h
 * @brief Basic Game Karakuri: 音響・ハプティクスサービス。
 *
 * ## 役割
 * - BGM の再生管理（フェードイン/アウト付きチャンネル切替え）。
 * - SE の再生（同時再生プール：最大 8ch）。
 * - モバイル端末のハプティクス（振動）制御。
 *
 * ## 設計ポリシー (karakuri 層)
 * - ミステリー固有の概念を一切知らない。
 * - ActionRunner と独立して動作し、mystery 層から同時呼び出しできる。
 * - AudioStreamPlayer を内部にプールとして持ち、シーン側に露出しない。
 *
 * ## 使い方
 * ```cpp
 * auto *snd = karakuri::SoundService::get_singleton();
 * snd->play_bgm("res://audio/bgm_investigation.ogg", 1.0f);
 * snd->play_se("res://audio/se_item_get.wav");
 * snd->vibrate(2, 0.3f);  // 強度2、0.3秒
 * ```
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class SoundService : public godot::Node {
  GDCLASS(SoundService, godot::Node)

private:
  static SoundService *singleton_;

  static constexpr int SE_POOL_SIZE = 8;

  /// @brief BGM 専用プレイヤー (index 0: current, index 1: next for crossfade)
  godot::Node *bgm_players_[2] = {nullptr, nullptr};
  int bgm_active_ = 0;

  /// @brief SE プール
  godot::Node *se_pool_[SE_POOL_SIZE] = {};

  /// @brief 現在再生中の BGM パス（重複再生防止）
  godot::String current_bgm_path_;

  /// @brief BGM 基本音量 (dB)
  float bgm_volume_db_ = 0.0f;
  /// @brief SE 基本音量 (dB)
  float se_volume_db_ = 0.0f;

  static void _bind_methods();

  void ensure_players();
  godot::Node *get_free_se_player();

public:
  SoundService();
  ~SoundService() override;

  void _ready() override;

  static SoundService *get_singleton();

  // ------------------------------------------------------------------
  // BGM
  // ------------------------------------------------------------------

  /**
   * @brief BGM を再生する。既に同じトラックが再生中なら無視する。
   * @param stream_path res:// パス（.ogg / .wav 等）。
   * @param fade_sec    フェードイン秒数。0 なら即時切替え。
   */
  void play_bgm(const godot::String &stream_path, float fade_sec = 0.0f);

  /**
   * @brief BGM をフェードアウトして停止する。
   * @param fade_sec フェードアウト秒数。
   */
  void stop_bgm(float fade_sec = 0.0f);

  // ------------------------------------------------------------------
  // SE
  // ------------------------------------------------------------------

  /**
   * @brief SE を再生する（プールから空きチャンネルを使用）。
   * @param stream_path res:// パス。
   * @param volume_db   追加音量オフセット (dB)。0 で基本音量のみ。
   */
  void play_se(const godot::String &stream_path, float volume_db = 0.0f);

  // ------------------------------------------------------------------
  // ハプティクス
  // ------------------------------------------------------------------

  /**
   * @brief デバイスを振動させる（モバイル端末のみ有効）。
   * @param intensity  振動強度 (0: 弱, 1: 中, 2: 強 — Godot 内部定義に依存)
   * @param duration_sec 継続秒数。
   */
  void vibrate(int intensity, float duration_sec);

  // ------------------------------------------------------------------
  // 音量設定
  // ------------------------------------------------------------------
  void set_bgm_volume_db(float db);
  float get_bgm_volume_db() const;

  void set_se_volume_db(float db);
  float get_se_volume_db() const;
};

} // namespace karakuri

#endif // KARAKURI_SOUND_SERVICE_H
