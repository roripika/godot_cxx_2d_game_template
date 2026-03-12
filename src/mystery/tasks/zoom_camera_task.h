#ifndef MYSTERY_ZOOM_CAMERA_TASK_H
#define MYSTERY_ZOOM_CAMERA_TASK_H

/**
 * @file zoom_camera_task.h
 * @brief Mystery: ActionRunner の zoom エフェクトをタスクとしてラップする。
 *
 * ## 動作
 * - on_start()         : ActionRunner::play_effect("zoom", {...}) を呼ぶ。
 * - on_update(delta)   : 内部タイマーを進め duration_ を超えたら finished_ = true。
 * - complete_instantly(): カメラの zoom を直接ワープさせ即終了。
 *
 * ## 使い方（GDScript）
 * ```gdscript
 * var task = ZoomCameraTask.new()
 * task.set_action_runner_path(NodePath("../ActionRunner"))
 * task.set_camera_path(NodePath("Camera2D"))
 * task.set_target_zoom(Vector2(1.5, 1.5))
 * task.set_duration(0.6)
 * ```
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace mystery {

class ZoomCameraTask : public karakuri::TaskBase {
  GDCLASS(ZoomCameraTask, karakuri::TaskBase)

  godot::NodePath action_runner_path_;
  godot::NodePath camera_path_;
  godot::Vector2 target_zoom_ = godot::Vector2(1.0f, 1.0f);
  double duration_ = 0.5;
  double elapsed_ = 0.0;

  bool started_ = false;

protected:
  static void _bind_methods();

public:
  ZoomCameraTask() = default;
  ~ZoomCameraTask() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル (ABI v1)
  // ------------------------------------------------------------------

  karakuri::TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;

  // ------------------------------------------------------------------
  // プロパティ
  // ------------------------------------------------------------------
  void set_action_runner_path(const godot::NodePath &path);
  godot::NodePath get_action_runner_path() const;

  void set_camera_path(const godot::NodePath &path);
  godot::NodePath get_camera_path() const;

  void set_target_zoom(const godot::Vector2 &zoom);
  godot::Vector2 get_target_zoom() const;

  void set_duration(double secs);
  double get_duration() const;
};

} // namespace mystery

#endif // MYSTERY_ZOOM_CAMERA_TASK_H
