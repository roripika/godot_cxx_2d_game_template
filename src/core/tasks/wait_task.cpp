#include "wait_task.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void WaitTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_duration"), &WaitTask::get_duration);
  ClassDB::bind_method(D_METHOD("mark_signal_received"), &WaitTask::mark_signal_received);
}

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

TaskResult WaitTask::execute(double delta) {
  if (!started_) {
    elapsed_ = 0.0;
    signal_received_ = false;
    started_ = true;
  }

  // シグナル待機モード: duration_ == 0.0 のとき mark_signal_received() を待つ
  if (duration_ <= 0.0) {
    return signal_received_ ? TaskResult::Success : TaskResult::Waiting;
  }

  // タイマーモード
  elapsed_ += delta;
  return (elapsed_ >= duration_) ? TaskResult::Success : TaskResult::Yielded;
}

godot::Error WaitTask::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("duration")) {
    duration_ = (double)spec["duration"];
  } else {
    // duration がない場合はシグナル待機モードとみなす
    duration_ = 0.0;
  }
  return godot::OK;
}

void WaitTask::complete_instantly() {
  elapsed_ = duration_;
  signal_received_ = true;
}

void WaitTask::mark_signal_received() {
  signal_received_ = true;
}

// ------------------------------------------------------------------
// プロパティ
// ------------------------------------------------------------------

double WaitTask::get_duration() const {
  return duration_;
}

} // namespace karakuri
