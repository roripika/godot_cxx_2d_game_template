#include "task_base.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void TaskBase::_bind_methods() {
  ClassDB::bind_method(D_METHOD("on_start"), &TaskBase::on_start);
  ClassDB::bind_method(D_METHOD("on_update", "delta"), &TaskBase::on_update);
  ClassDB::bind_method(D_METHOD("is_finished"), &TaskBase::is_finished);
  ClassDB::bind_method(D_METHOD("complete_instantly"), &TaskBase::complete_instantly);
}

void TaskBase::on_start() {
  // 基底実装は何もしない。サブクラスでオーバーライドする。
}

void TaskBase::on_update(double /*delta*/) {
  // 基底実装は何もしない。
}

bool TaskBase::is_finished() const {
  return finished_;
}

void TaskBase::complete_instantly() {
  finished_ = true;
}

} // namespace karakuri
