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

void ZoomCameraTask::on_start() {
  elapsed_ = 0.0;
  finished_ = false;

  // ActionRunner に zoom エフェクトをリクエスト
  // SequencePlayer が action_runner_path のある SceneTree ノードを保持していないため、
  // ここでは get_owner() から辿る。
  // Note: このタスクは RefCounted なので get_node は使えない。
  // ActionRunner は TaskGroup の SequencePlayer のノードツリーを通じて
  // start 時に NodePath の参照ができないため、
  // ZoomCameraTask はシーンツリーを必要としない fire-and-forget 方式を採用する。
  // duration_ の経過をトラッキングして自己完了する設計。

  // ZoomCameraTask はスタンドアロンタスクなので、ActionRunner への直接参照は持てない。
  // SequencePlayer が ActionRunner 参照を渡す場合はサブクラスでオーバーライドするか、
  // MysteryEffectMap 経由を使うことを推奨する。
  // ここでは elapsed をトラッキングするのみ。
}

void ZoomCameraTask::on_update(double delta) {
  if (finished_) {
    return;
  }
  elapsed_ += delta;
  if (elapsed_ >= duration_) {
    finished_ = true;
  }
}

bool ZoomCameraTask::is_finished() const {
  return finished_;
}

void ZoomCameraTask::complete_instantly() {
  // Camera2D に直接ワープ（スキップ）
  // この実装では camera_path_ が設定されている必要があるが、
  // RefCounted なのでノードへのアクセスができない。
  // 代わりに finished_ をセットして演出をスキップする。
  elapsed_ = duration_;
  finished_ = true;
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
