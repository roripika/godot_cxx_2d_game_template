#include "sequence_player.h"

#include "../scenario/scenario_runner.h"
#include "../services/action_runner.h"
#include "../services/sound_service.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void SequencePlayer::_bind_methods() {
  ClassDB::bind_method(D_METHOD("add_task", "task"), &SequencePlayer::add_task);
  ClassDB::bind_method(D_METHOD("clear_tasks"), &SequencePlayer::clear_tasks);
  ClassDB::bind_method(D_METHOD("start"), &SequencePlayer::start);
  ClassDB::bind_method(D_METHOD("skip_all"), &SequencePlayer::skip_all);
  ClassDB::bind_method(D_METHOD("is_running"), &SequencePlayer::is_running);
  ClassDB::bind_method(D_METHOD("create_snapshot"), &SequencePlayer::create_snapshot);
  ClassDB::bind_method(D_METHOD("rollback_to_last_snapshot"), &SequencePlayer::rollback_to_last_snapshot);

  ClassDB::bind_method(D_METHOD("set_scenario_runner_path", "path"),
                       &SequencePlayer::set_scenario_runner_path);
  ClassDB::bind_method(D_METHOD("get_scenario_runner_path"),
                       &SequencePlayer::get_scenario_runner_path);

  ClassDB::bind_method(D_METHOD("set_action_runner_path", "path"),
                       &SequencePlayer::set_action_runner_path);
  ClassDB::bind_method(D_METHOD("get_action_runner_path"),
                       &SequencePlayer::get_action_runner_path);

  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "scenario_runner_path"),
               "set_scenario_runner_path", "get_scenario_runner_path");
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "action_runner_path"),
               "set_action_runner_path", "get_action_runner_path");

  ADD_SIGNAL(MethodInfo("sequence_finished"));
  ADD_SIGNAL(MethodInfo("rollback_performed"));
}

// ------------------------------------------------------------------
// _process
// ------------------------------------------------------------------

void SequencePlayer::_process(double delta) {
  if (!running_) {
    return;
  }

  if (current_task_index_ >= task_queue_.size()) {
    finish_sequence();
    return;
  }

  Ref<TaskBase> current = task_queue_[current_task_index_];
  if (!current.is_valid()) {
    advance();
    return;
  }

  if (!started_current_) {
    current->on_start();
    started_current_ = true;
  }

  current->on_update(delta);

  if (current->is_finished()) {
    advance();
  }
}

// ------------------------------------------------------------------
// タスク管理
// ------------------------------------------------------------------

void SequencePlayer::add_task(const Ref<TaskBase> &task) {
  if (task.is_valid()) {
    task_queue_.append(task);
  }
}

void SequencePlayer::clear_tasks() {
  task_queue_.clear();
  current_task_index_ = 0;
  started_current_ = false;
  running_ = false;
}

// ------------------------------------------------------------------
// 再生制御
// ------------------------------------------------------------------

void SequencePlayer::start() {
  if (task_queue_.is_empty()) {
    finish_sequence();
    return;
  }
  current_task_index_ = 0;
  started_current_ = false;
  running_ = true;
}

void SequencePlayer::skip_all() {
  if (!running_) {
    return;
  }
  // 現在のタスクを含む残り全タスクを即完了
  for (int i = current_task_index_; i < task_queue_.size(); ++i) {
    Ref<TaskBase> task = task_queue_[i];
    if (task.is_valid()) {
      if (i == current_task_index_ && !started_current_) {
        task->on_start();
      }
      task->complete_instantly();
    }
  }
  finish_sequence();
}

bool SequencePlayer::is_running() const {
  return running_;
}

// ------------------------------------------------------------------
// スナップショット / ロールバック
// ------------------------------------------------------------------

void SequencePlayer::create_snapshot() {
  String scene_id;
  int action_index = 0;

  ScenarioRunner *sr = find_scenario_runner();
  if (sr != nullptr) {
    scene_id = sr->get_current_scene_id();
    action_index = sr->get_pending_action_index();
  }

  Ref<StateSnapshot> snap = StateSnapshot::capture(scene_id, action_index);
  rollback_stack_.append(snap);
}

void SequencePlayer::rollback_to_last_snapshot() {
  if (rollback_stack_.is_empty()) {
    UtilityFunctions::push_warning("SequencePlayer: rollback_stack is empty.");
    return;
  }

  // ActionRunner を止める
  ActionRunner *ar = find_action_runner();
  if (ar != nullptr) {
    ar->stop_all();
  }

  // BGM フェードアウト
  SoundService *ss = SoundService::get_singleton();
  if (ss != nullptr) {
    ss->stop_bgm(0.5f);
  }

  // スナップショットを取り出す（スタックから pop）
  Ref<StateSnapshot> snap = rollback_stack_[rollback_stack_.size() - 1];
  rollback_stack_.resize(rollback_stack_.size() - 1);

  if (!snap.is_valid()) {
    return;
  }

  // FlagService + ItemService を復元
  snap->restore_services();

  // ScenarioRunner をスナップショット時の位置に戻す
  ScenarioRunner *sr = find_scenario_runner();
  if (sr != nullptr) {
    sr->restore_to(snap->get_scene_id(), snap->get_command_index());
  }

  // キューをリセットし、実行停止
  current_task_index_ = 0;
  started_current_ = false;
  running_ = false;

  emit_signal("rollback_performed");
}

// ------------------------------------------------------------------
// NodePath プロパティ
// ------------------------------------------------------------------

void SequencePlayer::set_scenario_runner_path(const NodePath &path) {
  scenario_runner_path_ = path;
}

NodePath SequencePlayer::get_scenario_runner_path() const {
  return scenario_runner_path_;
}

void SequencePlayer::set_action_runner_path(const NodePath &path) {
  action_runner_path_ = path;
}

NodePath SequencePlayer::get_action_runner_path() const {
  return action_runner_path_;
}

// ------------------------------------------------------------------
// プライベートヘルパー
// ------------------------------------------------------------------

ScenarioRunner *SequencePlayer::find_scenario_runner() const {
  if (!scenario_runner_path_.is_empty()) {
    return Object::cast_to<ScenarioRunner>(get_node_or_null(scenario_runner_path_));
  }
  return nullptr;
}

ActionRunner *SequencePlayer::find_action_runner() const {
  if (!action_runner_path_.is_empty()) {
    return Object::cast_to<ActionRunner>(get_node_or_null(action_runner_path_));
  }
  return nullptr;
}

SoundService *SequencePlayer::find_sound_service() const {
  return SoundService::get_singleton();
}

void SequencePlayer::advance() {
  ++current_task_index_;
  started_current_ = false;
}

void SequencePlayer::finish_sequence() {
  running_ = false;
  emit_signal("sequence_finished");
}

} // namespace karakuri
