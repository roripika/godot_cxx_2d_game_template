#ifndef KARAKURI_ACTION_REGISTRY_H
#define KARAKURI_ACTION_REGISTRY_H

/**
 * @file action_registry.h
 * @brief Karakuri Kernel Architecture v2.0: 安全な Task ファクトリ。
 *
 * ## 設計方針 (Architecture v2.0)
 * - ClassDBSingleton::instantiate の文字列依存を廃止し、std::function<TaskBase*()> を用いた型安全なファクトリへ移行。
 * - TaskSpec を用いた Typed Scenario IR を導入。
 * - バリデーション失敗は Fail-Fast: null を返し、ロード段階でエラーを確定させる。
 */

#include <functional>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "tasks/task_base.h"
#include "tasks/task_spec.h"

namespace karakuri {

class ActionRegistry : public godot::Object {
  GDCLASS(ActionRegistry, godot::Object)

  static ActionRegistry *singleton_;

  /// アクション名 → TaskBase 生成ファクトリ関数のマッピング
  godot::HashMap<godot::String, std::function<TaskBase*()>> typed_registry_;

protected:
  static void _bind_methods();
  void init_builtin_actions();

public:
  ActionRegistry();
  ~ActionRegistry() override;

  static ActionRegistry *get_singleton();

  // ------------------------------------------------------------------
  // 登録 API
  // ------------------------------------------------------------------

  /**
   * @brief アクション名と C++ タスククラスを紐付ける型安全なファクトリ登録。
   *
   * @tparam T           TaskBase を継承した C++ クラス
   * @param action_name  YAML/JSON に書くアクション名 (例: "dialogue")
   */
  template <typename T>
  void register_action_class(const godot::String &action_name) {
    typed_registry_[action_name] = []() -> TaskBase* {
      return memnew(T);
    };
    godot::UtilityFunctions::print("[ActionRegistry] registered factory for: \"", action_name, "\"");
  }

  /**
   * @brief (非推奨) GDScript用バインディング。C++側では register_action_class<T>() を使用する。
   */
  void register_action_deprecated(const godot::String &action_name, const godot::String &class_name);

  // ------------------------------------------------------------------
  // 生成 API
  // ------------------------------------------------------------------

  /**
   * @brief TaskSpec を受け取り、対応するクラスを生成後、即座に validate_and_setup() を呼び出す。
   * エラーがあれば詳細をログに書き出し、null を返す (Fail-Fast)。
   *
   * @param spec シナリオロード時に展開された型付きIR (TaskSpec)
   * @return Ref<TaskBase> 検証に成功した実行可能なタスク、失敗時は null
   */
  godot::Ref<TaskBase> compile_task(const TaskSpec &spec);

  /** @brief 登録されているアクション名の一覧を返す（デバッグ用）。 */
  godot::Array get_registered_actions() const;

  /** @brief 指定のアクションが登録されているか確認する。 */
  bool has_action(const godot::String &action_name) const;
};

} // namespace karakuri

#endif // KARAKURI_ACTION_REGISTRY_H
