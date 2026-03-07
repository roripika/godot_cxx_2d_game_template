#include "mystery_trigger.h"

#include "../core/logic/condition_evaluator.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void MysteryTrigger::_bind_methods() {
  ClassDB::bind_method(D_METHOD("check"), &MysteryTrigger::check);
  ClassDB::bind_method(D_METHOD("check_and_notify"),
                       &MysteryTrigger::check_and_notify);

  ClassDB::bind_method(D_METHOD("set_condition_json", "d"),
                       &MysteryTrigger::set_condition_json);
  ClassDB::bind_method(D_METHOD("get_condition_json"),
                       &MysteryTrigger::get_condition_json);
  ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "condition_json"),
               "set_condition_json", "get_condition_json");

  ClassDB::bind_method(D_METHOD("set_auto_check", "value"),
                       &MysteryTrigger::set_auto_check);
  ClassDB::bind_method(D_METHOD("get_auto_check"),
                       &MysteryTrigger::get_auto_check);
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_check"),
               "set_auto_check", "get_auto_check");

  ADD_SIGNAL(MethodInfo("condition_met"));
  ADD_SIGNAL(MethodInfo("condition_unmet"));
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------
void MysteryTrigger::_ready() {
  set_process(auto_check_);
  // 初期状態を設定し、起動直後の誤シグナルを防ぐ
  if (!condition_json_.is_empty()) {
    last_result_ = karakuri::ConditionEvaluator::evaluate(condition_json_);
  }
  initialized_ = true;
}

void MysteryTrigger::_process([[maybe_unused]] double delta) {
  if (!auto_check_)
    return;
  check_and_notify();
}

// ---------------------------------------------------------------------------
// 条件評価
// ---------------------------------------------------------------------------
bool MysteryTrigger::check() {
  if (condition_json_.is_empty())
    return false;
  return karakuri::ConditionEvaluator::evaluate(condition_json_);
}

void MysteryTrigger::check_and_notify() {
  bool current = check();
  if (!initialized_) {
    last_result_ = current;
    initialized_ = true;
    return;
  }
  if (current == last_result_)
    return;

  last_result_ = current;
  if (current) {
    emit_signal("condition_met");
    UtilityFunctions::print("[MysteryTrigger] condition_met");
  } else {
    emit_signal("condition_unmet");
    UtilityFunctions::print("[MysteryTrigger] condition_unmet");
  }
}

// ---------------------------------------------------------------------------
// プロパティ
// ---------------------------------------------------------------------------
void MysteryTrigger::set_condition_json(const Dictionary &d) {
  condition_json_ = d;
}
Dictionary MysteryTrigger::get_condition_json() const { return condition_json_; }

void MysteryTrigger::set_auto_check(bool value) {
  auto_check_ = value;
  if (is_inside_tree())
    set_process(value);
}
bool MysteryTrigger::get_auto_check() const { return auto_check_; }

} // namespace mystery
