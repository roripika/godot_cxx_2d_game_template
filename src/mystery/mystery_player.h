#ifndef MYSTERY_PLAYER_H
#define MYSTERY_PLAYER_H

/**
 * @file mystery_player.h
 * @brief Mystery template: InputService を使って動くプレイヤーキャラクター。
 *
 * karakuri::BaseEntity を継承し、各 physics frame で
 * karakuri::InputService::get_move_direction() から入力を受け取る。
 *
 * 入力デバイスの差異（Keyboard / Joypad / Touch）は InputService に
 * 完全に隠蔽されており、このクラスは「どこから動けと言われたか」を
 * 一切知らない。
 */

#include "../core/entities/base_entity.h"
#include <godot_cpp/variant/vector2.hpp>

namespace mystery {

class MysteryPlayer : public karakuri::BaseEntity {
  GDCLASS(MysteryPlayer, karakuri::BaseEntity)

private:
  float interact_cooldown_ = 0.0f;  ///< @brief 調査ボタンの連打防止タイマー

  static void _bind_methods();

protected:
  void _notification(int p_what);

public:
  MysteryPlayer();
  ~MysteryPlayer() override = default;

  /**
   * @brief 調査・インタラクトアクション ("interact") が
   *        このフレームにトリガーされたら true を返す。
   */
  bool is_interacting() const;
};

} // namespace mystery

#endif // MYSTERY_PLAYER_H
