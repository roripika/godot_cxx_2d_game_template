#include "transition_object_task.h"
#include "../scenario/scenario_runner.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void TransitionObjectTask::_bind_methods() {}

TaskResult TransitionObjectTask::execute() {
  if (runner_ == nullptr) return TaskResult::Failed;

  if (!finished_) {
    // ScenarioRunner の transition_manager 等を使用して演出を実行するロジックが必要。
    // 現状は ScenarioRunner が直接演出をハンドルしているか、
    // TransitionManager に委譲している想定。
    
    // とりあえず ScenarioRunner にシグナルを投げさせるか、
    // 既存のメンバを操作する。
    
    // NOTE: 実際には TransitionManager に渡すべきだが、
    // 旧 ScenarioRunner の transition ロジックを Task 化する。
    
    // TODO: 実装
    // 一旦成功として進める（演出なしだが動作は継続させるため）
    // 本来はここで演出開始し、終了を待つ (TaskResult::Waiting)。
    UtilityFunctions::print("[TransitionObjectTask] Executing: ", params_);
    finished_ = true;
  }

  return TaskResult::Success;
}

Error TransitionObjectTask::validate_and_setup(const TaskSpec &spec) {
  TransitionObjectTaskSpec ts;
  const Dictionary &payload = spec.payload;

  if (!payload.has("target")) {
    UtilityFunctions::push_error("TransitionObjectTask: 'target' key is missing from spec.");
    return ERR_INVALID_DATA;
  }
  
  ts.params = payload;
  params_ = ts.params;
  return OK;
}

void TransitionObjectTask::complete_instantly() {
  finished_ = true;
}

} // namespace karakuri
