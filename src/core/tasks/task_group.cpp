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

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

TaskResult TaskGroup::execute(double delta) {
  if (tasks_.is_empty()) {
    return TaskResult::Success;
  }

  // 初回実行時に完了フラグを初期化
  if (completed_flags_.size() != tasks_.size()) {
    completed_flags_.resize(tasks_.size());
    for (int i = 0; i < completed_flags_.size(); ++i) {
      completed_flags_[i] = false;
    }
  }

  bool all_done = true;
  for (int i = 0; i < tasks_.size(); ++i) {
    if (completed_flags_[i]) {
      continue;
    }

    Ref<TaskBase> task = tasks_[i];
    if (task.is_valid()) {
      TaskResult res = task->execute(delta);
      if (res == TaskResult::Success) {
        completed_flags_[i] = true;
      } else if (res == TaskResult::Failed) {
        return TaskResult::Failed;
      } else {
        all_done = false;
      }
    } else {
      completed_flags_[i] = true;
    }
  }

  return all_done ? TaskResult::Success : TaskResult::Waiting;
}

godot::Error TaskGroup::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("tasks")) {
    Array arr = spec["tasks"];
    for (int i = 0; i < arr.size(); ++i) {
      add_task(arr[i]);
    }
  }
  return godot::OK;
}

void TaskGroup::complete_instantly() {
  for (int i = 0; i < tasks_.size(); ++i) {
    Ref<TaskBase> task = tasks_[i];
    if (task.is_valid()) {
      task->complete_instantly();
    }
  }
  // 全て完了したものとする
  completed_flags_.resize(tasks_.size());
  for (int i = 0; i < completed_flags_.size(); ++i) {
    completed_flags_[i] = true;
  }
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
  completed_flags_.clear();
  finished_ = false;
}

int TaskGroup::get_task_count() const {
  return tasks_.size();
}

} // namespace karakuri
