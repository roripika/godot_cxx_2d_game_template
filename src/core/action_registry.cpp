#include "action_registry.h"

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
  register_action("dialogue", "DialogueTask");
  register_action("choice", "ChoiceTask");
  register_action("wait", "WaitTask");
  register_action("goto", "GotoTask");
  register_action("set_flag", "SetFlagTask");
  register_action("if_flag", "IfFlagTask");
  register_action("if_has_items", "IfHasItemsTask");
  register_action("transition_object", "TransitionObjectTask");
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
                       &ActionRegistry::register_action);
  ClassDB::bind_method(D_METHOD("compile_task", "spec"),
                       &ActionRegistry::compile_task);
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

void ActionRegistry::register_action(const String &action_name,
                                      const String &class_name) {
  registry_[action_name] = class_name;
  UtilityFunctions::print("[ActionRegistry] registered: \"", action_name,
                          "\" → \"", class_name, "\"");
}

// ------------------------------------------------------------------
// 生成 API
// ------------------------------------------------------------------

Ref<TaskBase> ActionRegistry::compile_task(const Dictionary &spec) {
  if (!spec.has("action")) {
    UtilityFunctions::push_error("[ActionRegistry] 'action' key is missing from spec.");
    return Ref<TaskBase>();
  }
  
  String action_name = spec["action"];

  if (!registry_.has(action_name)) {
    UtilityFunctions::push_error(
        String("[ActionRegistry] 未登録のアクション: \"") + action_name +
        "\". register_action() で登録してください。");
    return Ref<TaskBase>();
  }

  const String class_name = registry_[action_name];
  Variant v = ClassDBSingleton::get_singleton()->instantiate(class_name);
  if (v.get_type() != Variant::OBJECT) {
    UtilityFunctions::push_error(
        String("[ActionRegistry] instantiate() が非オブジェクトを返しました: \"") +
        class_name + "\"");
    return Ref<TaskBase>();
  }

  Object *obj = v;
  TaskBase *task = Object::cast_to<TaskBase>(obj);
  if (task == nullptr) {
    UtilityFunctions::push_error(
        String("[ActionRegistry] \"") + class_name +
        "\" は TaskBase のサブクラスではありません。");
    return Ref<TaskBase>();
  }

  Ref<TaskBase> task_ref(task);
  
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
  return registry_.keys();
}

bool ActionRegistry::has_action(const String &action_name) const {
  return registry_.has(action_name);
}

} // namespace karakuri
