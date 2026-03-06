#ifndef KARAKURI_ACTION_RUNNER_H
#define KARAKURI_ACTION_RUNNER_H

/**
 * @file action_runner.h
 * @brief Basic Game Karakuri: 汎用演出ランナー。
 *
 * ## 役割
 * 画面シェイク・ズーム・フェード・フラッシュ・スローモーション等の
 * 演出ステップをキューで管理し、Tween を用いて順次または並列実行する。
 *
 * ## 設計ポリシー (karakuri 層)
 * - ミステリー固有の概念（証拠品・矛盾等）を一切知らない。
 * - EffectType をビットフラグとして組合せることで並列演出に対応。
 * - camera_path / overlay_path を設定しない場合はログのみ (安全フォールバック)。
 *
 * ## Godot シーンでの使い方
 * 1. ActionRunner ノードを任意の位置に配置。
 * 2. インスペクタで camera_path (Camera2D) と overlay_path (ColorRect) を設定。
 * 3. GDScript or C++ から play_effect("shake", {intensity:12, duration:0.4}) を呼ぶ。
 *
 * ## シグナル
 * - effect_sequence_finished() : play_sequence() の全ステップ完了時
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class ActionRunner : public godot::Node {
  GDCLASS(ActionRunner, godot::Node)

private:
  godot::NodePath camera_path_;
  godot::NodePath overlay_path_;

  /// @brief play_sequence() が進行中か
  bool sequence_running_ = false;
  /// @brief 現在実行中のシーケンスステップ配列 [{type, params}, ...]
  godot::Array sequence_steps_;
  int sequence_index_ = 0;
  /// @brief 現在のステップが完了するまでの残り秒数（parallel ステップ用）
  double step_wait_ = 0.0;

  static void _bind_methods();

  void run_next_step();
  void apply_shake(const godot::Dictionary &params);
  void apply_zoom(const godot::Dictionary &params);
  void apply_fade(const godot::Dictionary &params);
  void apply_flash(const godot::Dictionary &params);
  void apply_slow_motion(const godot::Dictionary &params);

  godot::Node *resolve(const godot::NodePath &path) const;

public:
  ActionRunner() = default;
  ~ActionRunner() override = default;

  void _process(double delta) override;

  // ------------------------------------------------------------------
  // 演出 API
  // ------------------------------------------------------------------

  /**
   * @brief 単発演出を即時実行する（non-blocking）。
   * @param type "shake" / "zoom" / "fade" / "flash" / "slow_motion"
   * @param params 演出パラメータ Dictionary (intensity, duration, value 等)
   */
  void play_effect(const godot::String &type, const godot::Dictionary &params);

  /**
   * @brief 演出ステップ配列を順次実行する。
   * 全ステップ完了後に effect_sequence_finished シグナルを発火。
   * @param steps [{ type: "shake", params: {...} }, ...] の Array
   */
  void play_sequence(const godot::Array &steps);

  /**
   * @brief 実行中の全演出を即時停止する。
   */
  void stop_all();

  bool get_sequence_running() const { return sequence_running_; }

  // ------------------------------------------------------------------
  // プロパティ
  // ------------------------------------------------------------------
  void set_camera_path(const godot::NodePath &path);
  godot::NodePath get_camera_path() const;

  void set_overlay_path(const godot::NodePath &path);
  godot::NodePath get_overlay_path() const;
};

} // namespace karakuri

#endif // KARAKURI_ACTION_RUNNER_H
