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
// Lifecycle
// ---------------------------------------------------------------------------

void MysteryPlayer::_ready() {
  // BaseEntity コンストラクタで set_physics_process(true) は呼ばれているが、
  // ここで明示することでサブクラスが上書きしても確実に有効になる。
  set_physics_process(true);

  // InputService がシングルトンとして取得できるか起動時に確認する。
  karakuri::InputService *svc = karakuri::InputService::get_singleton();
  if (!svc) {
    godot::UtilityFunctions::print(
        "[MysteryPlayer] WARNING: InputService not found! "
        "Make sure InputService is registered as an autoload in project.godot.");
  } else {
    godot::UtilityFunctions::print("[MysteryPlayer] InputService OK.");
  }
}

// ---------------------------------------------------------------------------
// Physics process
// ---------------------------------------------------------------------------

void MysteryPlayer::_notification(int p_what) {
  if (p_what == NOTIFICATION_PHYSICS_PROCESS) {
    // 1. 先に入力を取得して BaseEntity のメンバを更新する
    karakuri::InputService *input_svc = karakuri::InputService::get_singleton();

    if (input_svc) {
      Vector2 dir = input_svc->get_move_direction();
      set_movement_input(dir);

      // デバッグ: 入力があるときだけログを出す
      if (dir.length_squared() > 0.01f) {
        // UtilityFunctions::print(String("[MysteryPlayer] moving: ") + dir);
      }
    } else {
      static bool warned = false;
      if (!warned) {
        godot::UtilityFunctions::print(
            "[MysteryPlayer] Warning: InputService singleton not found!");
        warned = true;
      }
    }

    // 2. その後で親の処理（apply_movement = move_and_slide）を呼ぶ
    // これにより、今フレーム取得した入力が即座に反映される。
    karakuri::BaseEntity::_notification(p_what);

    // インタラクトクールダウンを減らす
    if (interact_cooldown_ > 0.0f) {
      interact_cooldown_ -=
          static_cast<float>(get_physics_process_delta_time());
    }
  } else {
    // PHYSICS_PROCESS 以外はそのまま親に流す
    karakuri::BaseEntity::_notification(p_what);
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
