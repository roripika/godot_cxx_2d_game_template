#ifndef KARAKURI_CONDITION_EVALUATOR_H
#define KARAKURI_CONDITION_EVALUATOR_H

/**
 * @file condition_evaluator.h
 * @brief Basic Game Karakuri: JSON (Dictionary) 形式の条件式を評価する論理判定エンジン。
 *
 * ## 目的
 * YAML / JSON で記述したシナリオ条件を C++ コードを一切変えずに評価できるようにする。
 * AI 生成シナリオや外部プランナーが書いた条件データをそのまま実行できるデータ駆動設計。
 *
 * ## サポートする DSL
 *
 * ### AND 条件
 * ```json
 * { "and": [ <cond1>, <cond2>, ... ] }
 * ```
 * 全ての要素が true のとき true。
 *
 * ### OR 条件
 * ```json
 * { "or": [ <cond1>, <cond2>, ... ] }
 * ```
 * いずれか 1 つが true のとき true。
 *
 * ### NOT 条件
 * ```json
 * { "not": <cond> }
 * ```
 * 内側の条件が false のとき true。
 *
 * ### フラグ存在チェック (truthy 判定)
 * ```json
 * { "flag": "has_knife" }
 * ```
 * `KarakuriGameState::get_flag("has_knife")` が truthy (bool true / 0 でない int / 空でない String) のとき true。
 *
 * ### 等値チェック
 * ```json
 * { "eq": ["phase", 2] }
 * ```
 * `get_flag("phase") == 2` のとき true。
 *
 * ## 使い方（GDScript）
 * ```gdscript
 * var cond = { "and": [{ "flag": "is_raining" }, { "not": { "flag": "has_umbrella" } }] }
 * if ConditionEvaluator.evaluate(cond):
 *     print("濡れる！")
 * ```
 *
 * ## 使い方（C++）
 * ```cpp
 * Dictionary cond; cond["flag"] = "has_knife";
 * bool result = karakuri::ConditionEvaluator::evaluate(cond);
 * ```
 */

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace karakuri {

class ConditionEvaluator : public godot::Object {
  GDCLASS(ConditionEvaluator, godot::Object)

protected:
  static void _bind_methods();

public:
  /**
   * @brief Dictionary 形式の条件式を評価して真偽値を返す。
   *
   * スタック再帰で and / or / not / flag / eq をサポート。
   * 未知のキーは false として扱う。
   */
  static bool evaluate(const godot::Dictionary &condition);
};

} // namespace karakuri

#endif // KARAKURI_CONDITION_EVALUATOR_H
