#ifndef KARAKURI_CONDITION_EVALUATOR_H
#define KARAKURI_CONDITION_EVALUATOR_H

/**
 * @file condition_evaluator.h
 * @brief Basic Game Karakuri: JSON 駆動の条件評価エンジン。
 *
 * ## 役割
 * - Dictionary（JSON）形式の条件式を再帰的に評価し true/false を返す。
 * - `FlagService` の現在値を参照することで、コード変更なしに
 *   シナリオデータだけで複雑な論理判定を表現できる。
 *
 * ## サポートする構文
 *
 * ### Leaf 判定（基本）
 * ```json
 * { "flag": "is_cabinet_opened", "op": "==", "value": true }
 * { "flag": "player_level",      "op": ">=", "value": 5    }
 * ```
 * `is` はエイリアス（`op: "=="` の省略形）:
 * ```json
 * { "flag": "has_key", "is": true }
 * ```
 *
 * ### item_owned — ItemService との連携
 * ```json
 * { "item_owned": "blood_knife" }
 * ```
 *
 * ### 複合条件
 * ```json
 * { "all": [ ...conditions ] }   // AND
 * { "any": [ ...conditions ] }   // OR
 * { "not": { ...condition }  }   // NOT
 * ```
 *
 * ## 使い方
 * ```cpp
 * Dictionary cond = ...;  // JSON から変換済み
 * bool result = karakuri::ConditionEvaluator::evaluate(cond);
 * ```
 */

#include "../services/flag_service.h"

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace karakuri {

class ConditionEvaluator {
public:
  /**
   * @brief 条件 Dictionary を評価して結果を返す（静的メソッド）。
   * @param condition 条件式の Dictionary または Array。
   * @param flags     参照するフラグストア。nullptr の場合 FlagService::get_singleton() を使う。
   * @return 条件が満たされれば true。
   */
  static bool evaluate(const godot::Variant &condition,
                       FlagService *flags = nullptr);

private:
  static bool eval_recursive(const godot::Variant &node, FlagService *flags);

  /// @brief Leaf 判定: { flag, op, value } または { flag, is }
  static bool eval_leaf(const godot::Dictionary &d, FlagService *flags);

  /// @brief 演算子文字列で 2値を比較する
  static bool compare(const godot::Variant &lhs, const godot::String &op,
                      const godot::Variant &rhs);
};

} // namespace karakuri

#endif // KARAKURI_CONDITION_EVALUATOR_H
