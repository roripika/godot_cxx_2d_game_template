#ifndef KARAKURI_ACTION_REGISTRY_H
#define KARAKURI_ACTION_REGISTRY_H

/**
 * @file action_registry.h
 * @brief Basic Game Karakuri: アクション名 → Godot クラス名 の動的ディスパッチ登録簿。
 *
 * ## 目的
 * YAML / JSON の `{"action": "add_evidence", "evidence_id": "knife"}` 形式のデータから
 * C++ クラスを動的に生成・実行できるようにする。
 * Core 層は Mystery 層のヘッダを一切 include しない（依存逆転防止）。
 *
 * ## 登録フロー
 * ```
 * [Mystery 初期化時]
 *   MysteryGameState::_ready()
 *     → ActionRegistry::register_action("add_evidence", "TaskAddEvidence")
 *
 * [シナリオ実行時]
 *   ScenarioRunner::execute_single_action({"action":"add_evidence","evidence_id":"knife"})
 *     → ActionRegistry::create_task("add_evidence")   // ClassDBSingleton::instantiate
 *     → task->set("evidence_id", "knife")              // Godot リフレクションで注入
 *     → task->on_start()
 * ```
 *
 * ## アーキテクチャ境界
 * - このクラスは src/core/ にのみ置く。
 * - src/mystery/ のヘッダを一切 include しない。
 * - ClassDBSingleton::instantiate() でクラス名から Object を生成するため、
 *   登録されたクラス名が Godot に正しく register_class されている必要がある。
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

  // ------------------------------------------------------------------
  // 生成 API
  // ------------------------------------------------------------------

  /**
   * @brief アクション名から TaskBase インスタンスを動的に生成して返す。
   *
   * 内部では ClassDBSingleton::instantiate() によりクラス名からオブジェクトを生成し、
   * TaskBase にキャストして Ref<> で返す。
   * 未登録のアクション名の場合は null Ref を返す。
   *
   * @param action_name  register_action() で登録したアクション名
   * @return Ref<TaskBase> (未登録または cast 失敗時は null)
   */
  godot::Ref<TaskBase> create_task(const godot::String &action_name);

  /** @brief 登録されているアクション名の一覧を返す（デバッグ用）。 */
  godot::Array get_registered_actions() const;

  /** @brief 指定のアクションが登録されているか確認する。 */
  bool has_action(const godot::String &action_name) const;
};

} // namespace karakuri

#endif // KARAKURI_ACTION_REGISTRY_H
