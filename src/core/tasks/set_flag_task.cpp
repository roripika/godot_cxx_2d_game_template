#include "set_flag_task.h"
#include "../world_state.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void SetFlagTask::_bind_methods() {}

// "<ns>:<scope_str>:<key>" を分解して {ns, scope, key} に展開するヘルパー
namespace {
  struct FlagAddr { String ns; int scope; String key; };
  FlagAddr parse_flag_expr(const String &expr) {
    FlagAddr a{ "core", WorldState::SCOPE_GLOBAL, expr };
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

TaskResult SetFlagTask::execute(double /*delta*/) {
  auto *ws = WorldState::get_singleton();
  if (ws && !key_.is_empty()) {
    auto a = parse_flag_expr(key_);
    ws->set_state(a.ns, a.scope, a.key, value_);
  }
  return TaskResult::Success;
}

Error SetFlagTask::validate_and_setup(const Dictionary &spec) {
  if (spec.has("key")) {
    key_ = spec["key"];
    value_ = spec.has("value") ? spec["value"] : Variant(true);
  } else if (spec.has("name")) {
    key_ = spec["name"];
    value_ = spec.has("value") ? spec["value"] : Variant(true);
  } else {
    return ERR_INVALID_DATA;
  }
  return OK;
}

void SetFlagTask::complete_instantly() {
  execute(0.0);
}

} // namespace karakuri
