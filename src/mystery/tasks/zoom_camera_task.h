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
#include "../../core/tasks/task_spec.h"

namespace mystery {

struct ZoomCameraTaskSpec {
  godot::Vector2 target_zoom;
  double duration = 0.5;
  godot::NodePath camera_path;
  godot::NodePath action_runner_path;
};


class ZoomCameraTask : public karakuri::TaskBase {
  GDCLASS(ZoomCameraTask, karakuri::TaskBase)

  godot::NodePath action_runner_path_;
  godot::NodePath camera_path_;
  godot::Vector2 target_zoom_ = godot::Vector2(1.0f, 1.0f);
  double duration_ = 0.5;
  double target_time_ = 0.0;

  bool started_ = false;

protected:
  static void _bind_methods();

public:
  ZoomCameraTask() = default;
  ~ZoomCameraTask() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル (ABI v1)
  // ------------------------------------------------------------------

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;

  // ------------------------------------------------------------------
  // プロパティ
  // ------------------------------------------------------------------
  godot::NodePath get_action_runner_path() const;

  godot::NodePath get_camera_path() const;

  godot::Vector2 get_target_zoom() const;

  double get_duration() const;
};

} // namespace mystery

#endif // MYSTERY_ZOOM_CAMERA_TASK_H
