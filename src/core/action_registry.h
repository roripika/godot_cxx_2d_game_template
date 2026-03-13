#ifndef KARAKURI_ACTION_REGISTRY_H
#define KARAKURI_ACTION_REGISTRY_H

/**
 * @file action_registry.h
 * @brief Karakuri Kernel ABI v1.5: アクション名 → C++ クラスのマッピングと
 *        ロード時コンパイル（型付き IR 生成）を担うファクトリ。
 *
 * ## 設計方針 (ABI v1.5)
 * - 動的プロパティ注入 (task->set(k, v)) を禁止。
 * - compile_task(spec) でクラスの instantiate と validate_and_setup() を一括実行。
 * - バリデーション失敗は Fail-Fast: null を返し、ロード段階でエラーを確定させる。
 * - 実行フェーズ (_process) は検証済みタスクを execute() するだけの状態機械となる。
 *
 * ## アーキテクチャ境界
 * - src/core/ にのみ置く。src/mystery/ を include しない（依存逆転防止）。
 * - Mystery 層の拡張タスクは MysteryGameState::_ready() で register_action() する。
 */

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/ref.hpp>

#include "tasks/task_base.h"

namespace karakuri {

class ActionRegistry : public godot::Object {
  GDCLASS(ActionRegistry, godot::Object)

  static ActionRegistry *singleton_;

  /// アクション名 → Godot クラス名 のマッピング
  godot::Dictionary registry_;

protected:
  static void _bind_methods();
  void init_builtin_actions();

public:
  ActionRegistry();
  ~ActionRegistry() override;

  /** @brief シングルトンを返す。 */
  static ActionRegistry *get_singleton();

  // ------------------------------------------------------------------
  // 登録 API
  // ------------------------------------------------------------------

  /**
   * @brief アクション名と Godot クラス名を紐付ける。
   *
   * @param action_name  YAML/JSON に書くアクション名 (例: "add_evidence")
   * @param class_name   GDExtension に register_class された C++ クラス名 (例: "TaskAddEvidence")
   *
   * Mystery 層の初期化時（_ready() 等で）に一度だけ呼ぶこと。
   */
  void register_action(const godot::String &action_name,
                       const godot::String &class_name);

  /**
   * @brief 辞書(spec)を受け取り、対応するクラスを Instantiate 後、即座に validate_and_setup() を呼び出す。
   * エラーがあれば詳細をログに書き出し、null を返す (Fail-Fast)。
   *
   * @param spec YAML/JSON で記述された1つのアクション定義 (必須キー: "action")
   * @return Ref<TaskBase> 検証に成功した実行可能なタスク、失敗時は null
   */
  godot::Ref<TaskBase> compile_task(const godot::Dictionary &spec);

  /** @brief 登録されているアクション名の一覧を返す（デバッグ用）。 */
  godot::Array get_registered_actions() const;

  /** @brief 指定のアクションが登録されているか確認する。 */
  bool has_action(const godot::String &action_name) const;
};

} // namespace karakuri

#endif // KARAKURI_ACTION_REGISTRY_H
