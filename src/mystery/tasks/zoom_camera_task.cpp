#include "zoom_camera_task.h"

#include "../../core/services/action_runner.h"

#include <godot_cpp/classes/camera2d.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void ZoomCameraTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_action_runner_path", "path"),
                       &ZoomCameraTask::set_action_runner_path);
  ClassDB::bind_method(D_METHOD("get_action_runner_path"),
                       &ZoomCameraTask::get_action_runner_path);

  ClassDB::bind_method(D_METHOD("set_camera_path", "path"),
                       &ZoomCameraTask::set_camera_path);
  ClassDB::bind_method(D_METHOD("get_camera_path"),
                       &ZoomCameraTask::get_camera_path);

  ClassDB::bind_method(D_METHOD("set_target_zoom", "zoom"),
                       &ZoomCameraTask::set_target_zoom);
  ClassDB::bind_method(D_METHOD("get_target_zoom"),
                       &ZoomCameraTask::get_target_zoom);

  ClassDB::bind_method(D_METHOD("set_duration", "secs"),
                       &ZoomCameraTask::set_duration);
  ClassDB::bind_method(D_METHOD("get_duration"),
                       &ZoomCameraTask::get_duration);

  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "action_runner_path"),
               "set_action_runner_path", "get_action_runner_path");
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_path"),
               "set_camera_path", "get_camera_path");
  ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "target_zoom"),
               "set_target_zoom", "get_target_zoom");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"),
               "set_duration", "get_duration");
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

void ZoomCameraTask::set_action_runner_path(const NodePath &path) {
  action_runner_path_ = path;
}
NodePath ZoomCameraTask::get_action_runner_path() const {
  return action_runner_path_;
}

void ZoomCameraTask::set_camera_path(const NodePath &path) {
  camera_path_ = path;
}
NodePath ZoomCameraTask::get_camera_path() const {
  return camera_path_;
}

void ZoomCameraTask::set_target_zoom(const Vector2 &zoom) {
  target_zoom_ = zoom;
}
Vector2 ZoomCameraTask::get_target_zoom() const {
  return target_zoom_;
}

void ZoomCameraTask::set_duration(double secs) {
  duration_ = secs;
}
double ZoomCameraTask::get_duration() const {
  return duration_;
}

} // namespace mystery
