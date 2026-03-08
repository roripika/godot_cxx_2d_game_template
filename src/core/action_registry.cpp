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
  ClassDB::bind_method(D_METHOD("create_task", "action_name"),
                       &ActionRegistry::create_task);
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

Ref<TaskBase> ActionRegistry::create_task(const String &action_name) {
  if (!registry_.has(action_name)) {
    UtilityFunctions::push_warning(
        String("[ActionRegistry] 未登録のアクション: \"") + action_name +
        "\". register_action() で登録してください。");
    return Ref<TaskBase>();
  }

  const String class_name = registry_[action_name];

  // ClassDBSingleton::instantiate() でクラス名からインスタンスを動的生成する。
  // RefCounted サブクラスの場合、返り値の Variant が内部で Ref<> を保持 (refcount=1)。
  // Object* を取り出した後 Ref<TaskBase> でラップすることで refcount=2 になり、
  // Variant がスコープを出た時点で refcount=1 に戻る（メモリリークなし）。
  Variant v = ClassDBSingleton::get_singleton()->instantiate(class_name);
  if (v.get_type() != Variant::OBJECT) {
    UtilityFunctions::push_error(
        String("[ActionRegistry] instantiate() が非オブジェクトを返しました: \"") +
        class_name + "\"");
    return Ref<TaskBase>();
  }

  Object *obj = v;  // Variant::operator Object*() — Variant の参照は保持したまま
  TaskBase *task = Object::cast_to<TaskBase>(obj);
  if (task == nullptr) {
    UtilityFunctions::push_error(
        String("[ActionRegistry] \"") + class_name +
        "\" は TaskBase のサブクラスではありません。");
    return Ref<TaskBase>();  // Variant がスコープを出て自動解放
  }

  // Ref<> ラップで refcount: 1 (Variant) + 1 (Ref) = 2
  // 関数終了時 v のデストラクタで 1 に戻り、呼び出し元が 1 ref を保持 ✓
  return Ref<TaskBase>(task);
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
