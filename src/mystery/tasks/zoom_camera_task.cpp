#include "zoom_camera_task.h"

#include "../../core/services/action_runner.h"

#include <godot_cpp/classes/camera2d.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void ZoomCameraTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_action_runner_path"),
                       &ZoomCameraTask::get_action_runner_path);
  ClassDB::bind_method(D_METHOD("get_camera_path"),
                       &ZoomCameraTask::get_camera_path);
  ClassDB::bind_method(D_METHOD("get_target_zoom"),
                       &ZoomCameraTask::get_target_zoom);
  ClassDB::bind_method(D_METHOD("get_duration"),
                       &ZoomCameraTask::get_duration);
}

// ------------------------------------------------------------------
// ライフサイクル
// ------------------------------------------------------------------

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

karakuri::TaskResult ZoomCameraTask::execute(double delta) {
  if (!started_) {
    elapsed_ = 0.0;
    started_ = true;
    // zoom エフェクト本体のリクエストは本来ここで行うが、
    // 現状は elapsed_ のトラッキングのみ行う（fire-and-forget 互換）。
  }

  elapsed_ += delta;
  if (elapsed_ >= duration_) {
    return karakuri::TaskResult::Success;
  }
  return karakuri::TaskResult::Yielded;
}

godot::Error ZoomCameraTask::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("target_zoom")) {
    target_zoom_ = spec["target_zoom"];
  }
  if (spec.has("duration")) {
    duration_ = spec["duration"];
  }
  if (spec.has("camera_path")) {
    camera_path_ = spec["camera_path"];
  }
  if (spec.has("action_runner_path")) {
    action_runner_path_ = spec["action_runner_path"];
  }
  
  return godot::OK;
}

void ZoomCameraTask::complete_instantly() {
  elapsed_ = duration_;
}

// ------------------------------------------------------------------
// プロパティ
// ------------------------------------------------------------------

NodePath ZoomCameraTask::get_action_runner_path() const {
  return action_runner_path_;
}

NodePath ZoomCameraTask::get_camera_path() const {
  return camera_path_;
}

Vector2 ZoomCameraTask::get_target_zoom() const {
  return target_zoom_;
}

double ZoomCameraTask::get_duration() const {
  return duration_;
}

} // namespace mystery
