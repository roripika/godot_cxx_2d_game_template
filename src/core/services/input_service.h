#ifndef KARAKURI_INPUT_SERVICE_H
#define KARAKURI_INPUT_SERVICE_H

/**
 * @file input_service.h
 * @brief Basic Game Karakuri: クロスプラットフォーム入力抽象化サービス。
 *
 * Keyboard / Joypad / Touch (Virtual Joystick) を透過的に扱い、
 * ゲームロジックへ統一した Vector2 / bool インターフェースを提供する。
 *
 * 使用する Input Map アクション名:
 *   move_left, move_right, move_up, move_down  — 4方向移動
 *   interact                                   — 調査・決定ボタン
 *
 * タッチ対応:
 *   GDScript の VirtualJoystick UI が毎フレーム
 *   InputService.set_virtual_joystick(direction) を呼ぶことで、
 *   get_move_direction() がそれを自動合成する。
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace karakuri {

class InputService : public godot::Node {
  GDCLASS(InputService, godot::Node)

private:
  static InputService *singleton_;

  /// @brief GDScript の VirtualJoystick UI から注入されるタッチ入力 (-1..1)
  godot::Vector2 virtual_joystick_{};

  static void _bind_methods();

public:
  InputService();
  ~InputService() override;

  static InputService *get_singleton();

  // ------------------------------------------------------------------
  // 移動入力
  // ------------------------------------------------------------------

  /**
   * @brief 今フレームの移動方向を正規化した Vector2 で返す。
   *
   * 優先順位:
   *   1. キーボード / Joypad (Godot Input Map "move_*" アクション)
   *   2. タッチ (virtual_joystick_ が非ゼロの場合に合成)
   *
   * @return 長さ 0〜1 の方向ベクトル。
   */
  godot::Vector2 get_move_direction() const;

  // ------------------------------------------------------------------
  // アクション入力
  // ------------------------------------------------------------------

  /**
   * @brief 指定アクションがこのフレームに押されたか。
   * @param action_name Input Map のアクション名。
   */
  bool is_action_just_pressed(const godot::String &action_name) const;

  /**
   * @brief 指定アクションが押し続けられているか。
   */
  bool is_action_pressed(const godot::String &action_name) const;

  // ------------------------------------------------------------------
  // タッチ (VirtualJoystick) インターフェース
  // ------------------------------------------------------------------

  /**
   * @brief GDScript VirtualJoystick UI から毎フレーム呼ばれる。
   * @param direction 正規化済み方向ベクトル (-1,1) 範囲推奨。ゼロで無効化。
   */
  void set_virtual_joystick(const godot::Vector2 &direction);

  /**
   * @brief 現在セットされているタッチ入力を返す（デバッグ用）。
   */
  godot::Vector2 get_virtual_joystick() const;
};

} // namespace karakuri

#endif // KARAKURI_INPUT_SERVICE_H
