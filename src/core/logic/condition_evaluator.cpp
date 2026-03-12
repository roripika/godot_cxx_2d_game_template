#include "logic/condition_evaluator.h"

#include "services/item_service.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

// ---------------------------------------------------------------------------
// パブリックエントリポイント
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// 内部ヘルパー: フラグ式を WorldState から解決する
// フォーマット: "<ns>:<scope_str>:<key>"  or  "<key>"(後方互換)
// ---------------------------------------------------------------------------
Variant ConditionEvaluator::resolve_flag(const String &flag_expr) {
  auto *ws = WorldState::get_singleton();
  if (!ws) return Variant();

  if (flag_expr.contains(":")) {
    PackedStringArray parts = flag_expr.split(":");
    if (parts.size() >= 3) {
      String ns        = parts[0];
      String scope_str = parts[1];
      String key       = parts[2];
      int scope = WorldState::SCOPE_GLOBAL;
      if      (scope_str == "session") scope = WorldState::SCOPE_SESSION;
      else if (scope_str == "scene")   scope = WorldState::SCOPE_SCENE;
      return ws->get_state(ns, scope, key);
    }
  }
  // 後方互換: 単純文字列 → namespace="core", scope=GLOBAL
  return ws->get_state("core", WorldState::SCOPE_GLOBAL, flag_expr);
}

bool ConditionEvaluator::evaluate(const Variant &condition) {
  return eval_recursive(condition);
}

// ---------------------------------------------------------------------------
// 再帰評価
// ---------------------------------------------------------------------------
bool ConditionEvaluator::eval_recursive(const Variant &node) {
  if (node.get_type() == Variant::DICTIONARY) {
    Dictionary d = node;

    // ── 複合条件 ─────────────────────────────────────────────
    // all: AND
    if (d.has("all")) {
      Variant v = d["all"];
      if (v.get_type() != Variant::ARRAY)
        return false;
      Array arr = v;
      for (int i = 0; i < arr.size(); ++i) {
        if (!eval_recursive(arr[i]))
          return false;
      }
      return true;
    }

    // any: OR
    if (d.has("any")) {
      Variant v = d["any"];
      if (v.get_type() != Variant::ARRAY)
        return false;
      Array arr = v;
      for (int i = 0; i < arr.size(); ++i) {
        if (eval_recursive(arr[i]))
          return true;
      }
      return false;
    }

    // not: NOT
    if (d.has("not")) {
      return !eval_recursive(d["not"]);
    }

    // ── 特殊 Leaf ──────────────────────────────────────────────
    // item_owned: ItemService との連携
    if (d.has("item_owned")) {
      String item_id = String(d["item_owned"]);
      auto *svc = ItemService::get_singleton();
      return svc && svc->has_item(item_id);
    }

    // ── 基本 Leaf ──────────────────────────────────────────────
    return eval_leaf(d);
  }

  // Array は暗黙 all として扱う
  if (node.get_type() == Variant::ARRAY) {
    Array arr = node;
    for (int i = 0; i < arr.size(); ++i) {
      if (!eval_recursive(arr[i]))
        return false;
    }
    return true;
  }

  // bool リテラル
  if (node.get_type() == Variant::BOOL) {
    return bool(node);
  }

  UtilityFunctions::push_warning(
      "ConditionEvaluator: unexpected node type in condition.");
  return false;
}

// ---------------------------------------------------------------------------
// Leaf 評価
// ---------------------------------------------------------------------------
bool ConditionEvaluator::eval_leaf(const Dictionary &d) {
  if (!d.has("flag")) {
    UtilityFunctions::push_warning(
        "ConditionEvaluator: leaf node has no 'flag' key.");
    return false;
  }

  String flag_expr = String(d["flag"]);
  Variant lhs = resolve_flag(flag_expr);

  // `is` ショートカット → op == でリダイレクト
  if (d.has("is")) {
    return compare(lhs, "==", d["is"]);
  }

  // 明示的な op + value
  if (d.has("op") && d.has("value")) {
    String op  = String(d["op"]);
    Variant rhs = d["value"];
    return compare(lhs, op, rhs);
  }

  UtilityFunctions::push_warning(
      "ConditionEvaluator: leaf for flag '" + flag_expr +
      "' has neither 'is' nor 'op'/'value'.");
  return false;
}

// ---------------------------------------------------------------------------
// 比較演算子
// ---------------------------------------------------------------------------
bool ConditionEvaluator::compare(const Variant &lhs, const String &op,
                                  const Variant &rhs) {
  if (op == "==" || op == "is") {
    return lhs == rhs;
  } else if (op == "!=" || op == "not") {
    return lhs != rhs;
  }

  // 数値比較 — Variant を double にキャスト
  double l = double(lhs);
  double r = double(rhs);
  if (op == ">")  return l >  r;
  if (op == "<")  return l <  r;
  if (op == ">=") return l >= r;
  if (op == "<=") return l <= r;

  UtilityFunctions::push_warning(
      "ConditionEvaluator: unknown operator '" + op + "'.");
  return false;
}

} // namespace karakuri
