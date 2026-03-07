#ifndef MYSTERY_TRIGGER_H
#define MYSTERY_TRIGGER_H

/**
 * @file mystery_trigger.h
 * @brief Mystery Layer: 条件充足時シグナル発火コンポーネント。
 *
 * ## 役割
 * - ConditionEvaluator を内包し、設定された条件が満たされたとき
 *   `condition_met` シグナルを発火する Node コンポーネント。
 * - auto_check = true にすると _process() で毎フレーム自動評価。
 * - 用例:「証拠品 A 取得済み」&「人物 B の証言聴取済み」 → 新調査ポイント出現
 *
 * ## Godot シーンでの使い方
 * 1. MysteryTrigger ノードを Area2D 等の子に配置。
 * 2. インスペクタで `condition_json` に条件 Dictionary を設定。
 * 3. `condition_met` シグナルを受けてノードの表示/非表示等を切り替える。
 *
 * ## シグナル
 * - condition_met()   : 条件が新たに満たされたとき
 * - condition_unmet() : 満たされていた条件が不成立に戻ったとき
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace mystery {

class MysteryTrigger : public godot::Node {
  GDCLASS(MysteryTrigger, godot::Node)

private:
  /// @brief 評価する条件式 Dictionary（インスペクタから設定）
  godot::Dictionary condition_json_;

  /// @brief true のとき _process() で毎フレーム自動評価
  bool auto_check_ = false;

  /// @brief 前回の評価結果（状態変化シグナルの重複防止用）
  bool last_result_ = false;

  /// @brief 初回評価済みフラグ（起動直後の偽陽性を防ぐ）
  bool initialized_ = false;

  static void _bind_methods();

public:
  MysteryTrigger() = default;
  ~MysteryTrigger() override = default;

  void _ready() override;
  void _process(double delta) override;

  // ------------------------------------------------------------------
  // 条件評価 API
  // ------------------------------------------------------------------

  /**
   * @brief 条件を手動で評価する。
   * @return 現在の condition_json_ が満たされていれば true。
   */
  bool check();

  /**
   * @brief 評価して結果が変化していればシグナルを発火する。
   */
  void check_and_notify();

  // ------------------------------------------------------------------
  // プロパティ
  // ------------------------------------------------------------------

  void set_condition_json(const godot::Dictionary &d);
  godot::Dictionary get_condition_json() const;

  void set_auto_check(bool value);
  bool get_auto_check() const;

  // ------------------------------------------------------------------
  // Signals
  // ------------------------------------------------------------------
  // condition_met()   — 条件が新たに満たされたとき
  // condition_unmet() — 条件が不成立に戻ったとき
};

} // namespace mystery

#endif // MYSTERY_TRIGGER_H
