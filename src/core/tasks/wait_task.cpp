#include "wait_task.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void WaitTask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_duration", "seconds"), &WaitTask::set_duration);
  ClassDB::bind_method(D_METHOD("get_duration"), &WaitTask::get_duration);

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"), "set_duration", "get_duration");
}

// ------------------------------------------------------------------
// ライフサイクル
// ------------------------------------------------------------------

void WaitTask::on_start() {
  elapsed_ = 0.0;
  signal_received_ = false;
}

void WaitTask::on_update(double delta) {
  if (signal_received_) {
    return;
  }
  elapsed_ += delta;
  if (elapsed_ >= duration_) {
    finished_ = true;
  }
}

bool WaitTask::is_finished() const {
  return finished_;
}

void WaitTask::complete_instantly() {
  elapsed_ = duration_;
  signal_received_ = true;
  finished_ = true;
}

// ------------------------------------------------------------------
// プロパティアクセサ
// ------------------------------------------------------------------

void WaitTask::set_duration(double seconds) {
  duration_ = seconds;
}

double WaitTask::get_duration() const {
  return duration_;
}

} // namespace karakuri
