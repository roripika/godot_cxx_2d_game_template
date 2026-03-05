#include "input_service.h"

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

InputService *InputService::singleton_ = nullptr;

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

InputService::InputService() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
}

InputService::~InputService() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

InputService *InputService::get_singleton() { return singleton_; }

// ---------------------------------------------------------------------------
// 移動入力
// ---------------------------------------------------------------------------

Vector2 InputService::get_move_direction() const {
  Input *input = Input::get_singleton();
  if (!input) {
    return virtual_joystick_;
  }

  // Keyboard & Joypad: Input Map "move_*" アクション経由で取得。
  // Godot の get_vector() は deadzone 処理・正規化も自動で行う。
  Vector2 physical = input->get_vector(
      "move_left", "move_right", "move_up", "move_down");

  // タッチ入力と合成（物理入力を優先）
  Vector2 result = physical.length_squared() > 0.01f
                       ? physical
                       : virtual_joystick_;

  // length() > 1 になる対角入力を正規化
  if (result.length_squared() > 1.0f) {
    result = result.normalized();
  }
  return result;
}

// ---------------------------------------------------------------------------
// アクション入力
// ---------------------------------------------------------------------------

bool InputService::is_action_just_pressed(const String &action_name) const {
  Input *input = Input::get_singleton();
  return input ? input->is_action_just_pressed(action_name) : false;
}

bool InputService::is_action_pressed(const String &action_name) const {
  Input *input = Input::get_singleton();
  return input ? input->is_action_pressed(action_name) : false;
}

// ---------------------------------------------------------------------------
// タッチ VirtualJoystick
// ---------------------------------------------------------------------------

void InputService::set_virtual_joystick(const Vector2 &direction) {
  virtual_joystick_ = direction;
}

Vector2 InputService::get_virtual_joystick() const {
  return virtual_joystick_;
}

// ---------------------------------------------------------------------------
// Godot bindings
// ---------------------------------------------------------------------------

void InputService::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_move_direction"),
                       &InputService::get_move_direction);
  ClassDB::bind_method(D_METHOD("is_action_just_pressed", "action_name"),
                       &InputService::is_action_just_pressed);
  ClassDB::bind_method(D_METHOD("is_action_pressed", "action_name"),
                       &InputService::is_action_pressed);
  ClassDB::bind_method(D_METHOD("set_virtual_joystick", "direction"),
                       &InputService::set_virtual_joystick);
  ClassDB::bind_method(D_METHOD("get_virtual_joystick"),
                       &InputService::get_virtual_joystick);
}

} // namespace karakuri
