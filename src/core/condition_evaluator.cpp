#include "condition_evaluator.h"

#include "karakuri_game_state.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void ConditionEvaluator::_bind_methods() {
  ClassDB::bind_static_method("ConditionEvaluator",
                              D_METHOD("evaluate", "condition"),
                              &ConditionEvaluator::evaluate);
}

// ------------------------------------------------------------------
// 内部ヘルパー: Variant の truthy 判定
// ------------------------------------------------------------------
static bool is_truthy(const Variant &v) {
  switch (v.get_type()) {
    case Variant::NIL:    return false;
    case Variant::BOOL:   return (bool)v;
    case Variant::INT:    return (int64_t)v != 0;
    case Variant::FLOAT:  return (double)v != 0.0;
    case Variant::STRING: return !((String)v).is_empty();
    default:              return true;  // Object, Array, Dictionary 等は存在すれば truthy
  }
}

// ------------------------------------------------------------------
// evaluate (再帰)
// ------------------------------------------------------------------
bool ConditionEvaluator::evaluate(const Dictionary &condition) {
  // ── and ──────────────────────────────────────────────────
  if (condition.has("and")) {
    const Variant &v = condition["and"];
    if (v.get_type() != Variant::ARRAY) {
      UtilityFunctions::push_error("[ConditionEvaluator] 'and' の値は Array でなければなりません。");
      return false;
    }
    const Array &arr = v;
    for (int i = 0; i < arr.size(); i++) {
      if (arr[i].get_type() != Variant::DICTIONARY) return false;
      if (!evaluate(Dictionary(arr[i]))) return false;
    }
    return true;
  }

  // ── or ───────────────────────────────────────────────────
  if (condition.has("or")) {
    const Variant &v = condition["or"];
    if (v.get_type() != Variant::ARRAY) {
      UtilityFunctions::push_error("[ConditionEvaluator] 'or' の値は Array でなければなりません。");
      return false;
    }
    const Array &arr = v;
    for (int i = 0; i < arr.size(); i++) {
      if (arr[i].get_type() != Variant::DICTIONARY) continue;
      if (evaluate(Dictionary(arr[i]))) return true;
    }
    return false;
  }

  // ── not ──────────────────────────────────────────────────
  if (condition.has("not")) {
    const Variant &v = condition["not"];
    if (v.get_type() != Variant::DICTIONARY) {
      UtilityFunctions::push_error("[ConditionEvaluator] 'not' の値は Dictionary でなければなりません。");
      return false;
    }
    return !evaluate(Dictionary(v));
  }

  // ── flag ─────────────────────────────────────────────────
  if (condition.has("flag")) {
    const String key = condition["flag"];
    KarakuriGameState *gs = KarakuriGameState::get_singleton();
    if (gs == nullptr) {
      UtilityFunctions::push_warning(
          "[ConditionEvaluator] KarakuriGameState が null です。'flag' 条件は false を返します。");
      return false;
    }
    return is_truthy(gs->get_flag(key));
  }

  // ── eq ───────────────────────────────────────────────────
  if (condition.has("eq")) {
    const Variant &v = condition["eq"];
    if (v.get_type() != Variant::ARRAY) {
      UtilityFunctions::push_error("[ConditionEvaluator] 'eq' の値は [flag_name, value] の Array でなければなりません。");
      return false;
    }
    const Array &arr = v;
    if (arr.size() < 2) {
      UtilityFunctions::push_error("[ConditionEvaluator] 'eq' には要素が 2 つ必要です: [flag_name, value]");
      return false;
    }
    const String key = arr[0];
    const Variant &expected = arr[1];
    KarakuriGameState *gs = KarakuriGameState::get_singleton();
    if (gs == nullptr) {
      UtilityFunctions::push_warning(
          "[ConditionEvaluator] KarakuriGameState が null です。'eq' 条件は false を返します。");
      return false;
    }
    return gs->get_flag(key) == expected;
  }

  // ── 未知のキー ────────────────────────────────────────────
  UtilityFunctions::push_warning(
      "[ConditionEvaluator] 未知の条件キーです。サポートされているキー: and / or / not / flag / eq");
  return false;
}

} // namespace karakuri
