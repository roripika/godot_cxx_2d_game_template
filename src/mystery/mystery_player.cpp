#include "mystery_player.h"

#include "../core/services/input_service.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

MysteryPlayer::MysteryPlayer() {
  // BaseEntity::speed のデフォルト (200) を mystery 向けに調整
  set_speed(180.0f);
}

// ---------------------------------------------------------------------------
// Godot bindings
// ---------------------------------------------------------------------------

void MysteryPlayer::_bind_methods() {
  ClassDB::bind_method(D_METHOD("is_interacting"),
                       &MysteryPlayer::is_interacting);
}

// ---------------------------------------------------------------------------
// Physics process
// ---------------------------------------------------------------------------

void MysteryPlayer::_notification(int p_what) {
  // 親クラスの _notification を先に呼ぶ (apply_movement が動く)
  karakuri::BaseEntity::_notification(p_what);

  if (p_what == NOTIFICATION_PHYSICS_PROCESS) {
    // InputService から移動方向を取得して BaseEntity に渡す
    karakuri::InputService *input_svc =
        karakuri::InputService::get_singleton();

    if (input_svc) {
      set_movement_input(input_svc->get_move_direction());
    }

    // インタラクトクールダウンを減らす
    if (interact_cooldown_ > 0.0f) {
      interact_cooldown_ -= static_cast<float>(get_physics_process_delta_time());
    }
  }
}

// ---------------------------------------------------------------------------
// Interaction
// ---------------------------------------------------------------------------

bool MysteryPlayer::is_interacting() const {
  karakuri::InputService *input_svc = karakuri::InputService::get_singleton();
  if (!input_svc) {
    return false;
  }
  return input_svc->is_action_just_pressed("interact");
}

} // namespace mystery
