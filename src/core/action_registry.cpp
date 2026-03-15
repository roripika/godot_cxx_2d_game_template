#include "action_registry.h"

// Built-in Tasks (Core)
#include "tasks/dialogue_task.h"
#include "tasks/choice_task.h"
#include "tasks/wait_task.h"
#include "tasks/goto_task.h"
#include "tasks/set_flag_task.h"
#include "tasks/if_flag_task.h"
#include "tasks/if_has_items_task.h"
#include "tasks/transition_object_task.h"

#include <godot_cpp/classes/class_db_singleton.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

ActionRegistry *ActionRegistry::singleton_ = nullptr;

ActionRegistry::ActionRegistry() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
  init_builtin_actions();
}

void ActionRegistry::init_builtin_actions() {
  register_action_class<DialogueTask>("dialogue");
  register_action_class<ChoiceTask>("choice");
  register_action_class<WaitTask>("wait");
  register_action_class<GotoTask>("goto");
  register_action_class<SetFlagTask>("set_flag");
  register_action_class<IfFlagTask>("if_flag");
  register_action_class<IfHasItemsTask>("if_has_items");
  register_action_class<TransitionObjectTask>("transition_object");
}

ActionRegistry::~ActionRegistry() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

ActionRegistry *ActionRegistry::get_singleton() {
  return singleton_;
}

void ActionRegistry::_bind_methods() {
  ClassDB::bind_method(D_METHOD("register_action", "action_name", "class_name"),
                       &ActionRegistry::register_action_deprecated);
  ClassDB::bind_method(D_METHOD("has_action", "action_name"),
                       &ActionRegistry::has_action);
  ClassDB::bind_method(D_METHOD("get_registered_actions"),
                       &ActionRegistry::get_registered_actions);

  ClassDB::bind_static_method("ActionRegistry",
                              D_METHOD("get_singleton"),
                              &ActionRegistry::get_singleton);
}

// ------------------------------------------------------------------
// 登録 API
// ------------------------------------------------------------------

void ActionRegistry::register_action_deprecated(const String &action_name,
                                                const String &class_name) {
  UtilityFunctions::push_error("[ActionRegistry] String-based registration is deprecated in v2.0. Use register_action_class<T>() from C++ instead. Failed to register: ", action_name);
}

// ------------------------------------------------------------------
// 生成 API
// ------------------------------------------------------------------

Ref<TaskBase> ActionRegistry::compile_task(const TaskSpec &spec) {
  if (spec.action.is_empty()) {
    UtilityFunctions::push_error("[ActionRegistry] Action spec is empty.");
    return Ref<TaskBase>();
  }

  String action_name = spec.action;

  if (!typed_registry_.has(action_name)) {
    UtilityFunctions::push_error(
        String("[ActionRegistry] 未登録のアクション: \"") + action_name +
        "\". register_action_class() で登録してください。");
    return Ref<TaskBase>();
  }

  // Factory関数を呼び出してインスタンスを生成
  TaskBase *task_ptr = typed_registry_[action_name]();
  if (task_ptr == nullptr) {
    UtilityFunctions::push_error(
        String("[ActionRegistry] タスクの生成に失敗しました: \"") + action_name + "\"");
    return Ref<TaskBase>();
  }

  Ref<TaskBase> task_ref(task_ptr);
  
  // バリデーションとセットアップ
  Error err = task_ref->validate_and_setup(spec);
  if (err != OK) {
    UtilityFunctions::push_error(
        String("[ActionRegistry] タスクの検証に失敗しました: \"") + action_name +
        "\" (Error: " + String::num(err) + ")");
    return Ref<TaskBase>(); // null を返すことでロード失敗を誘発
  }

  return task_ref;
}

// ------------------------------------------------------------------
// 参照 API
// ------------------------------------------------------------------

Array ActionRegistry::get_registered_actions() const {
  Array keys;
  for (const auto &E : typed_registry_) {
    keys.append(E.key);
  }
  return keys;
}

bool ActionRegistry::has_action(const String &action_name) const {
  return typed_registry_.has(action_name);
}

} // namespace karakuri
