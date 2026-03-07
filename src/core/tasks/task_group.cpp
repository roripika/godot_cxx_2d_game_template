#include "task_group.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void TaskGroup::_bind_methods() {
  ClassDB::bind_method(D_METHOD("add_task", "task"), &TaskGroup::add_task);
  ClassDB::bind_method(D_METHOD("clear_tasks"), &TaskGroup::clear_tasks);
  ClassDB::bind_method(D_METHOD("get_task_count"), &TaskGroup::get_task_count);
}

// ------------------------------------------------------------------
// ライフサイクル
// ------------------------------------------------------------------

void TaskGroup::on_start() {
  for (int i = 0; i < tasks_.size(); ++i) {
    Ref<TaskBase> task = tasks_[i];
    if (task.is_valid()) {
      task->on_start();
    }
  }
}

void TaskGroup::on_update(double delta) {
  for (int i = 0; i < tasks_.size(); ++i) {
    Ref<TaskBase> task = tasks_[i];
    if (task.is_valid() && !task->is_finished()) {
      task->on_update(delta);
    }
  }

  // 全タスク完了チェック
  bool all_done = true;
  for (int i = 0; i < tasks_.size(); ++i) {
    Ref<TaskBase> task = tasks_[i];
    if (task.is_valid() && !task->is_finished()) {
      all_done = false;
      break;
    }
  }
  if (all_done) {
    finished_ = true;
  }
}

bool TaskGroup::is_finished() const {
  if (tasks_.is_empty()) {
    return true;
  }
  return finished_;
}

void TaskGroup::complete_instantly() {
  for (int i = 0; i < tasks_.size(); ++i) {
    Ref<TaskBase> task = tasks_[i];
    if (task.is_valid() && !task->is_finished()) {
      task->complete_instantly();
    }
  }
  finished_ = true;
}

// ------------------------------------------------------------------
// タスク管理
// ------------------------------------------------------------------

void TaskGroup::add_task(const Ref<TaskBase> &task) {
  if (task.is_valid()) {
    tasks_.append(task);
  }
}

void TaskGroup::clear_tasks() {
  tasks_.clear();
  finished_ = false;
}

int TaskGroup::get_task_count() const {
  return tasks_.size();
}

} // namespace karakuri
