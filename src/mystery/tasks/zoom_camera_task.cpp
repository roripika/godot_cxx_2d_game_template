#include "zoom_camera_task.h"

#include "../../core/services/action_runner.h"
#include "../../core/kernel_clock.h"

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

karakuri::TaskResult ZoomCameraTask::execute() {
  auto *clock = karakuri::KernelClock::get_singleton();
  ERR_FAIL_NULL_V(clock, karakuri::TaskResult::Failed);

  if (!started_) {
    target_time_ = clock->now() + duration_;
    started_ = true;
    // zoom エフェクト本体のリクエストは本来ここで行うが、
    // 現状は target_time_ のトラッキングのみ行う（fire-and-forget 互換）。
  }

  if (clock->now() >= target_time_) {
    return karakuri::TaskResult::Success;
  }
  return karakuri::TaskResult::Yielded;
}

godot::Error ZoomCameraTask::validate_and_setup(const karakuri::TaskSpec &spec) {
  ZoomCameraTaskSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("target_zoom")) {
    ts.target_zoom = payload["target_zoom"];
  } else {
    ts.target_zoom = godot::Vector2(1.0f, 1.0f);
  }
  if (payload.has("duration")) {
    ts.duration = payload["duration"];
  }
  if (payload.has("camera_path")) {
    ts.camera_path = payload["camera_path"];
  }
  if (payload.has("action_runner_path")) {
    ts.action_runner_path = payload["action_runner_path"];
  }
  
  target_zoom_ = ts.target_zoom;
  duration_ = ts.duration;
  camera_path_ = ts.camera_path;
  action_runner_path_ = ts.action_runner_path;
  return godot::OK;
}

void ZoomCameraTask::complete_instantly() {
  auto *clock = karakuri::KernelClock::get_singleton();
  if (clock) {
    target_time_ = clock->now();
  }
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
