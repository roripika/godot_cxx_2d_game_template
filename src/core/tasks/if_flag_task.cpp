#include "if_flag_task.h"
#include "../scenario/scenario_runner.h"
#include "../world_state.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void IfFlagTask::_bind_methods() {}

// "<ns>:<scope_str>:<key>" を分解するヘルパー（set_flag_task と同形）
namespace {
  struct FlagAddr2 { String ns; int scope; String key; };
  FlagAddr2 parse_flag_expr2(const String &expr) {
    FlagAddr2 a{ "core", WorldState::SCOPE_GLOBAL, expr };
    if (expr.contains(":")) {
      PackedStringArray p = expr.split(":");
      if (p.size() >= 3) {
        a.ns  = p[0];
        a.key = p[2];
        if      (p[1] == "session") a.scope = WorldState::SCOPE_SESSION;
        else if (p[1] == "scene")   a.scope = WorldState::SCOPE_SCENE;
      }
    }
    return a;
  }
} // anonymous namespace

TaskResult IfFlagTask::execute() {
  if (runner_ == nullptr) return TaskResult::Failed;

  auto *ws = WorldState::get_singleton();
  bool result = false;
  if (ws) {
    auto a = parse_flag_expr2(key_);
    Variant current = ws->get_state(a.ns, a.scope, a.key);
    result = (current == expected_value_);
  }

  const Array &branch = result ? then_branch_ : else_branch_;
  if (!branch.is_empty()) {
    runner_->inject_steps(branch);
  }

  return TaskResult::Success;
}

Error IfFlagTask::validate_and_setup(const TaskSpec &spec) {
  IfFlagTaskSpec ts;
  const Dictionary &payload = spec.payload;

  if (payload.has("key")) {
    ts.key = payload["key"];
    ts.expected_value = payload.has("value") ? payload["value"] : Variant(true);
    ts.then_branch = payload.has("then") ? Array(payload["then"]) : Array();
    ts.else_branch = payload.has("else") ? Array(payload["else"]) : Array();
  } else {
    UtilityFunctions::push_error("IfFlagTask: 'key' is missing from spec.");
    return ERR_INVALID_DATA;
  }
  
  key_ = ts.key;
  expected_value_ = ts.expected_value;
  then_branch_ = ts.then_branch;
  else_branch_ = ts.else_branch;
  return OK;
}

void IfFlagTask::complete_instantly() {
  execute();
}

} // namespace karakuri
