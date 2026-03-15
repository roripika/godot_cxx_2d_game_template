#include "wait_task.h"

#include <godot_cpp/core/class_db.hpp>
#include "../kernel_clock.h"

using namespace godot;

namespace karakuri {

void WaitTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_duration"), &WaitTask::get_duration);
  ClassDB::bind_method(D_METHOD("mark_signal_received"), &WaitTask::mark_signal_received);
}

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

TaskResult WaitTask::execute() {
  auto *clock = KernelClock::get_singleton();
  ERR_FAIL_NULL_V(clock, TaskResult::Failed);

  if (!started_) {
    target_time_ = clock->now() + duration_;
    signal_received_ = false;
    started_ = true;
  }

  if (mode_ == WaitMode::Timer) {
    if (clock->now() >= target_time_) {
      return TaskResult::Success;
    }
    return TaskResult::Yielded;
  }

  if (mode_ == WaitMode::Signal) {
    return signal_received_ ? TaskResult::Success : TaskResult::Waiting;
  }

  return TaskResult::Yielded;
}

godot::Error WaitTask::validate_and_setup(const TaskSpec &spec) {
  WaitTaskSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("duration")) {
    ts.duration = (double)payload["duration"];
    mode_ = WaitMode::Timer;
  } else {
    // duration がない場合はシグナル待機モードとみなす
    ts.duration = 0.0;
    mode_ = WaitMode::Signal;
  }
  
  duration_ = ts.duration;
  return godot::OK;
}

void WaitTask::complete_instantly() {
  auto *clock = KernelClock::get_singleton();
  if (clock) {
    target_time_ = clock->now();
  }
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
