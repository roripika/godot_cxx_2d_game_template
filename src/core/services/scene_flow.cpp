#include "services/scene_flow.h"

#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
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
  ClassDB::bind_method(D_METHOD("return_to_title"), &SceneFlow::return_to_title);

  ClassDB::bind_method(D_METHOD("can_pop"), &SceneFlow::can_pop);
  ClassDB::bind_method(D_METHOD("get_current_path"),
                       &SceneFlow::get_current_path);
  ClassDB::bind_method(D_METHOD("get_current_params"),
                       &SceneFlow::get_current_params);
  ClassDB::bind_method(D_METHOD("get_stack_depth"),
                       &SceneFlow::get_stack_depth);
  ClassDB::bind_method(D_METHOD("clear_history"), &SceneFlow::clear_history);

  ClassDB::bind_method(D_METHOD("set_title_scene_path", "path"),
                       &SceneFlow::set_title_scene_path);
  ClassDB::bind_method(D_METHOD("get_title_scene_path"),
                       &SceneFlow::get_title_scene_path);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "title_scene_path",
                            PROPERTY_HINT_FILE, "*.tscn,*.scn"),
               "set_title_scene_path", "get_title_scene_path");

  // 遷移イベントシグナル
  ADD_SIGNAL(MethodInfo("scene_replaced",
                        PropertyInfo(Variant::STRING, "from_path"),
                        PropertyInfo(Variant::STRING, "to_path")));
  ADD_SIGNAL(MethodInfo("scene_pushed",
                        PropertyInfo(Variant::STRING, "from_path"),
                        PropertyInfo(Variant::STRING, "to_path")));
  ADD_SIGNAL(MethodInfo("scene_popped",
                        PropertyInfo(Variant::STRING, "from_path"),
                        PropertyInfo(Variant::STRING, "to_path")));

  // フェードイン/アウト演出フックシグナル
  // このシグナルを SceneFlowLayer.tscn（GDScript）に接続し、
  // フェードアウトアニメーションを再生する。Karakuri は見た目を知らない。
  ADD_SIGNAL(MethodInfo("transition_started",
                        PropertyInfo(Variant::STRING, "to_path")));
  ADD_SIGNAL(MethodInfo("transition_finished"));
}

// ------------------------------------------------------------------
// 内部ヘルパー
// ------------------------------------------------------------------

void SceneFlow::do_replace_scene(const String &path) {
  SceneTree *tree = get_tree();
  if (tree == nullptr) {
    UtilityFunctions::push_error(
        "[SceneFlow] do_replace_scene: SceneTree が null です。"
        "SceneFlow が Autoload として登録されていることを確認してください。");
    return;
  }

  // 遷移後にGDScript側のフェードアウトアニメーションをトリガーするため、先にシグナルを発火する。
  // SceneFlowLayer.tscn の AnimationPlayer が transition_started を受けフェードアウトを実行。
  // アニメーション完了後、GDScript が SceneFlow.do_finish_transition() を呼ぶ対応も可能。
  // ここではまず即座に遷移し、後に transition_finished を発火する。(フェードアウト待変わりなしの即時遷移モード)
  emit_signal("transition_started", path);

  Error err = tree->change_scene_to_file(path);
  if (err != Error::OK) {
    UtilityFunctions::push_error(
        String("[SceneFlow] シーンの読み込みに失敗しました: ") + path);
    return;
  }

  // 遷移完了シグナル：フェードイン開始のトリガー。
  // 注: 実際に新シーンがロードされるのは次フレームなので、
  //       GDScript 側では transition_started 後〬 1 フレーム待ってからフェードインするのが正しい。
  emit_signal("transition_finished");
}

// ------------------------------------------------------------------
// 遷移 API
// ------------------------------------------------------------------

void SceneFlow::replace_scene(const String &path, const Dictionary &params) {
  String from = current_path_;
  current_path_ = path;
  current_params_ = params;
  active_pushed_scene_ = nullptr;  // SceneTree が新シーンを管理する

  UtilityFunctions::print(
      String("[SceneFlow] replace: ") + from + String(" → ") + path);

  emit_signal("scene_replaced", from, path);
  do_replace_scene(path);
}

void SceneFlow::push_scene(const String &path, const Dictionary &params) {
  SceneTree *tree = get_tree();
  if (tree == nullptr) {
    UtilityFunctions::push_error(
        "[SceneFlow] push_scene: SceneTree が null です。Autoload 設定を確認してください。");
    return;
  }

  // ① 現在アクティブなシーンノードを取得する。
  //    初回 push なら SceneTree が管理しているシーンを使う。
  //    2 回目以降の push なら自前で追跡している active_pushed_scene_ を使う。
  Node *current = (active_pushed_scene_ != nullptr)
                      ? active_pushed_scene_
                      : tree->get_current_scene();

  // ② ルートから取り外す（queue_free しない！メモリ上に保持する）
  if (current != nullptr) {
    tree->get_root()->remove_child(current);
  }

  // ③ スタックに保存
  SceneRecord record;
  record.path            = current_path_;
  record.params          = current_params_;
  record.suspended_scene = current;
  history_stack_.push_back(record);

  // ④ 新しいシーンをロードしてインスタンス化
  Ref<PackedScene> packed = ResourceLoader::get_singleton()->load(path);
  if (packed.is_null()) {
    UtilityFunctions::push_error(
        String("[SceneFlow] push_scene: シーンのロードに失敗しました: ") + path);
    // ロールバック: 取り外したシーンを戻す
    if (current != nullptr) {
      tree->get_root()->add_child(current);
    }
    history_stack_.pop_back();
    return;
  }

  Node *new_scene = packed->instantiate();
  if (new_scene == nullptr) {
    UtilityFunctions::push_error(
        String("[SceneFlow] push_scene: instantiate() が null を返しました: ") + path);
    if (current != nullptr) {
      tree->get_root()->add_child(current);
    }
    history_stack_.pop_back();
    return;
  }

  // ⑤ ルートに追加してアクティブに設定
  tree->get_root()->add_child(new_scene);
  active_pushed_scene_ = new_scene;

  String from       = record.path;
  current_path_     = path;
  current_params_   = params;

  UtilityFunctions::print(
      String("[SceneFlow] push: ") + from + String(" → ") + path +
      String(" (stack depth: ") + String::num_int64((int64_t)history_stack_.size()) +
      String(")"));

  emit_signal("scene_pushed", from, path);
  emit_signal("transition_started", path);
  emit_signal("transition_finished");
}

void SceneFlow::pop_scene() {
  if (history_stack_.empty()) {
    UtilityFunctions::push_warning(
        "[SceneFlow] pop_scene: 履歴スタックが空です。can_pop() で確認してください。");
    return;
  }

  SceneTree *tree = get_tree();
  if (tree == nullptr) {
    UtilityFunctions::push_error(
        "[SceneFlow] pop_scene: SceneTree が null です。");
    return;
  }

  String from = current_path_;

  // ① 現在表示中のシーンを解放する
  if (active_pushed_scene_ != nullptr) {
    active_pushed_scene_->queue_free();
    active_pushed_scene_ = nullptr;
  }

  // ② スタックから直前のレコードを取り出す
  SceneRecord record = history_stack_.back();
  history_stack_.pop_back();

  current_path_   = record.path;
  current_params_ = record.params;

  // ③ suspend していたシーンノードをルートに再接続して復元する
  Node *restored = record.suspended_scene;
  if (restored != nullptr) {
    tree->get_root()->add_child(restored);
    active_pushed_scene_ = restored;
  } else {
    active_pushed_scene_ = nullptr;
  }

  UtilityFunctions::print(
      String("[SceneFlow] pop: ") + from + String(" → ") + current_path_ +
      String(" (stack depth: ") + String::num_int64((int64_t)history_stack_.size()) +
      String(")"));

  emit_signal("scene_popped", from, current_path_);
  emit_signal("transition_started", current_path_);
  emit_signal("transition_finished");
}

// ------------------------------------------------------------------
// 参照 API
// ------------------------------------------------------------------

bool SceneFlow::can_pop() const {
  return !history_stack_.empty();
}

String SceneFlow::get_current_path() const {
  return current_path_;
}

Dictionary SceneFlow::get_current_params() const {
  return current_params_;
}

int SceneFlow::get_stack_depth() const {
  return (int)history_stack_.size();
}

void SceneFlow::clear_history() {
  // スタックに保持されている全 suspend 済みシーンノードを解放する
  for (auto &record : history_stack_) {
    if (record.suspended_scene != nullptr) {
      record.suspended_scene->queue_free();
    }
  }
  history_stack_.clear();
  UtilityFunctions::print("[SceneFlow] clear_history: 履歴スタックをクリアしました。");
}

void SceneFlow::return_to_title() {
  if (title_scene_path_.is_empty()) {
    UtilityFunctions::push_warning(
        "[SceneFlow] return_to_title: title_scene_path が未設定です。"
        "インスペクタまたはコードから set_title_scene_path() で設定してください。");
    return;
  }
  String from = current_path_;

  // ① push_scene で表示中のシーンを解放する
  if (active_pushed_scene_ != nullptr) {
    active_pushed_scene_->queue_free();
    active_pushed_scene_ = nullptr;
  }

  // ② スタックに保持されている全 suspend 済みシーンを解放してクリア
  clear_history();

  current_path_   = title_scene_path_;
  current_params_ = Dictionary();

  UtilityFunctions::print(String("[SceneFlow] return_to_title: ") + from +
                          String(" → ") + title_scene_path_);

  emit_signal("scene_replaced", from, title_scene_path_);
  do_replace_scene(title_scene_path_);
}

// ------------------------------------------------------------------
// title_scene_path プロパティ
// ------------------------------------------------------------------

void SceneFlow::set_title_scene_path(const String &path) {
  title_scene_path_ = path;
}

String SceneFlow::get_title_scene_path() const {
  return title_scene_path_;
}

} // namespace karakuri
