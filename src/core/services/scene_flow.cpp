#include "services/scene_flow.h"

#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

SceneFlow *SceneFlow::singleton_ = nullptr;

SceneFlow::SceneFlow() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
}

SceneFlow::~SceneFlow() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

SceneFlow *SceneFlow::get_singleton() { return singleton_; }

// ------------------------------------------------------------------
// _bind_methods
// ------------------------------------------------------------------

void SceneFlow::_bind_methods() {
  ClassDB::bind_method(D_METHOD("replace_scene", "path", "params"),
                       &SceneFlow::replace_scene, DEFVAL(Dictionary()));
  ClassDB::bind_method(D_METHOD("push_scene", "path", "params"),
                       &SceneFlow::push_scene, DEFVAL(Dictionary()));
  ClassDB::bind_method(D_METHOD("pop_scene"), &SceneFlow::pop_scene);

  ClassDB::bind_method(D_METHOD("can_pop"), &SceneFlow::can_pop);
  ClassDB::bind_method(D_METHOD("get_current_path"),
                       &SceneFlow::get_current_path);
  ClassDB::bind_method(D_METHOD("get_current_params"),
                       &SceneFlow::get_current_params);
  ClassDB::bind_method(D_METHOD("get_stack_depth"),
                       &SceneFlow::get_stack_depth);
  ClassDB::bind_method(D_METHOD("clear_history"), &SceneFlow::clear_history);

  ADD_SIGNAL(MethodInfo("scene_replaced",
                        PropertyInfo(Variant::STRING, "from_path"),
                        PropertyInfo(Variant::STRING, "to_path")));
  ADD_SIGNAL(MethodInfo("scene_pushed",
                        PropertyInfo(Variant::STRING, "from_path"),
                        PropertyInfo(Variant::STRING, "to_path")));
  ADD_SIGNAL(MethodInfo("scene_popped",
                        PropertyInfo(Variant::STRING, "from_path"),
                        PropertyInfo(Variant::STRING, "to_path")));
}

// ------------------------------------------------------------------
// 内部ヘルパー
// ------------------------------------------------------------------

void SceneFlow::do_change_scene(const String &path) {
  SceneTree *tree = get_tree();
  if (tree == nullptr) {
    UtilityFunctions::push_error(
        "[SceneFlow] do_change_scene: SceneTree が null です。"
        "SceneFlow が Autoload として登録されていることを確認してください。");
    return;
  }
  Error err = tree->change_scene_to_file(path);
  if (err != Error::OK) {
    UtilityFunctions::push_error(
        String("[SceneFlow] シーンの読み込みに失敗しました: ") + path);
  }
}

// ------------------------------------------------------------------
// 遷移 API
// ------------------------------------------------------------------

void SceneFlow::replace_scene(const String &path, const Dictionary &params) {
  String from = current_path_;
  current_path_ = path;
  current_params_ = params;

  UtilityFunctions::print(
      String("[SceneFlow] replace: ") + from + String(" → ") + path);

  emit_signal("scene_replaced", from, path);
  do_change_scene(path);
}

void SceneFlow::push_scene(const String &path, const Dictionary &params) {
  String from = current_path_;

  // 現在のシーンパスをスタックに保存
  Dictionary entry;
  entry["path"] = current_path_;
  entry["params"] = current_params_;
  history_stack_.append(entry);

  current_path_ = path;
  current_params_ = params;

  UtilityFunctions::print(
      String("[SceneFlow] push: ") + from + String(" → ") + path +
      String(" (stack depth: ") + String::num(history_stack_.size()) +
      String(")"));

  emit_signal("scene_pushed", from, path);
  do_change_scene(path);
}

void SceneFlow::pop_scene() {
  if (history_stack_.is_empty()) {
    UtilityFunctions::push_warning(
        "[SceneFlow] pop_scene: 履歴スタックが空です。can_pop() で確認してください。");
    return;
  }

  String from = current_path_;

  // スタックから直前のエントリを取り出す
  Dictionary entry = history_stack_[history_stack_.size() - 1];
  history_stack_.resize(history_stack_.size() - 1);

  String to = String(entry.get("path", String("")));
  current_path_ = to;
  current_params_ = Dictionary(entry.get("params", Dictionary()));

  UtilityFunctions::print(
      String("[SceneFlow] pop: ") + from + String(" → ") + to +
      String(" (stack depth: ") + String::num(history_stack_.size()) +
      String(")"));

  emit_signal("scene_popped", from, to);
  do_change_scene(to);
}

// ------------------------------------------------------------------
// 参照 API
// ------------------------------------------------------------------

bool SceneFlow::can_pop() const {
  return !history_stack_.is_empty();
}

String SceneFlow::get_current_path() const {
  return current_path_;
}

Dictionary SceneFlow::get_current_params() const {
  return current_params_;
}

int SceneFlow::get_stack_depth() const {
  return history_stack_.size();
}

void SceneFlow::clear_history() {
  history_stack_.clear();
  UtilityFunctions::print("[SceneFlow] clear_history: 履歴スタックをクリアしました。");
}

} // namespace karakuri
