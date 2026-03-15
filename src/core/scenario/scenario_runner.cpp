#include "scenario_runner.h"
#include "kernel_clock.h"
#include "../action_registry.h"
#include "../logger/karakuri_logger.h"
#include "../logic/condition_evaluator.h"
#include "../services/save_service.h"
#include "../world_state.h"
#include "../yaml/yaml_lite.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

namespace {
// --- 匿名ヘルパー群 ---------------------------------------------------------
static Dictionary as_dict(const Variant &v) {
  return (v.get_type() == Variant::DICTIONARY) ? (Dictionary)v : Dictionary();
}
static Array as_array(const Variant &v) {
  return (v.get_type() == Variant::ARRAY) ? (Array)v : Array();
}
static String dict_get_string(const Dictionary &d, const String &key,
                              const String &def = "") {
  if (!d.has(key))
    return def;
  const Variant v = d[key];
  return (v.get_type() == Variant::STRING ||
          v.get_type() == Variant::STRING_NAME)
             ? String(v)
             : def;
}

// --- InlineHandlerTask: ActionHandler ラムダを TaskBase にラップする ----------
// GDCLASS 不使用 (Godot リフレクション不要の C++ 内部専用タスク)
class InlineHandlerTask : public TaskBase {
  ScenarioRunner::ActionHandler handler_;
  Variant payload_;
  bool done_ = false;

public:
  InlineHandlerTask(ScenarioRunner::ActionHandler h, Variant p)
      : handler_(std::move(h)), payload_(std::move(p)) {}

  TaskResult execute() override {
    if (!done_) {
      handler_(payload_);
      done_ = true;
    }
    return TaskResult::Success;
  }
  godot::Error validate_and_setup(const TaskSpec &) override { return godot::OK; }
  void complete_instantly() override { done_ = true; }
};

} // namespace

ScenarioRunner::ScenarioRunner() {}
ScenarioRunner::~ScenarioRunner() {}

void ScenarioRunner::register_action(const String &kind, ActionHandler handler) {
  action_handlers_[kind] = handler;
}

void ScenarioRunner::set_scenario_path(const String &path) {
  scenario_path_ = path;
}
String ScenarioRunner::get_scenario_path() const { return scenario_path_; }

bool ScenarioRunner::is_running() const {
  return is_executing_actions_ || waiting_for_choice_ ||
         waiting_for_dialogue_ || waiting_for_transition_ ||
         wait_remaining_sec_ > 0.0;
}

String ScenarioRunner::get_current_scene_id() const {
  return current_scene_id_;
}

int ScenarioRunner::get_pending_action_index() const {
  return pending_action_index_;
}

void ScenarioRunner::restore_to(const String &scene_id, int action_index) {
  // 実行中の待ち状態をクリア
  wait_remaining_sec_ = 0.0;
  waiting_for_choice_ = false;
  waiting_for_dialogue_ = false;
  waiting_for_transition_ = false;
  waiting_for_custom_action_ = false;
  is_executing_actions_ = false;

  // アクションインデックスを復元
  pending_action_index_ = action_index;

  // シーン遷移が必要なときのみ load_scene_by_id を呼ぶ
  if (!scene_id.is_empty() && scene_id != current_scene_id_) {
    load_scene_by_id(scene_id);
  }
}

void ScenarioRunner::complete_custom_action() {
  waiting_for_custom_action_ = false;
}

void ScenarioRunner::complete_transition() {
  waiting_for_transition_ = false;
  transition_start_time_ = 0.0;
}

void ScenarioRunner::request_transition(const godot::String &target_scene, const godot::Dictionary &params) {
  waiting_for_transition_ = true;
  transition_start_time_ = KernelClock::get_singleton()->now();
  emit_signal("transition_requested", target_scene, params);
}

void ScenarioRunner::inject_steps(const Array &steps) {
  if (steps.is_empty())
    return;

  Array compiled_steps;
  for (int i = 0; i < steps.size(); i++) {
    Ref<TaskBase> task = compile_action(steps[i]);
    if (task.is_valid()) {
      compiled_steps.append(task);
    }
  }

  if (compiled_steps.is_empty()) return;

  // pending_action_index_ は現在実行中のタスクを指している。
  // その直後に割込ませる。
  Array merged;
  for (int i = 0; i <= pending_action_index_ && i < pending_actions_.size(); ++i) {
    merged.append(pending_actions_[i]);
  }
  for (int i = 0; i < compiled_steps.size(); ++i) {
    merged.append(compiled_steps[i]);
  }
  for (int i = pending_action_index_ + 1; i < pending_actions_.size(); ++i) {
    merged.append(pending_actions_[i]);
  }
  pending_actions_ = merged;
}

void ScenarioRunner::_bind_methods() {
  ClassDB::bind_method(D_METHOD("load_scenario"),
                       &ScenarioRunner::load_scenario);
  ClassDB::bind_method(D_METHOD("load_scene_by_id", "scene_id"),
                       &ScenarioRunner::load_scene_by_id);
  ClassDB::bind_method(D_METHOD("complete_custom_action"),
                       &ScenarioRunner::complete_custom_action);
  ClassDB::bind_method(D_METHOD("complete_transition"),
                       &ScenarioRunner::complete_transition);
  ClassDB::bind_method(D_METHOD("inject_steps", "steps"),
                       &ScenarioRunner::inject_steps);
  ClassDB::bind_method(D_METHOD("get_current_scene_id"),
                       &ScenarioRunner::get_current_scene_id);
  ClassDB::bind_method(D_METHOD("get_pending_action_index"),
                       &ScenarioRunner::get_pending_action_index);
  ClassDB::bind_method(D_METHOD("restore_to", "scene_id", "action_index"),
                       &ScenarioRunner::restore_to);
  ClassDB::bind_method(D_METHOD("is_running"), &ScenarioRunner::is_running);

  ClassDB::bind_method(D_METHOD("advance_dialogue"),
                       &ScenarioRunner::advance_dialogue);
  ClassDB::bind_method(D_METHOD("submit_choice", "index"),
                       &ScenarioRunner::submit_choice);

  ClassDB::bind_method(D_METHOD("set_scenario_path", "path"),
                       &ScenarioRunner::set_scenario_path);
  ClassDB::bind_method(D_METHOD("get_scenario_path"),
                       &ScenarioRunner::get_scenario_path);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "scenario_path"),
               "set_scenario_path", "get_scenario_path");

  // Decoupling Signals
  ADD_SIGNAL(MethodInfo("transition_requested", PropertyInfo(Variant::STRING, "target_scene"), PropertyInfo(Variant::DICTIONARY, "params")));
  ADD_SIGNAL(MethodInfo("transition_failed", PropertyInfo(Variant::STRING, "reason")));
  ADD_SIGNAL(MethodInfo("dialogue_requested", PropertyInfo(Variant::STRING, "speaker"), PropertyInfo(Variant::STRING, "text")));
  ADD_SIGNAL(MethodInfo("choices_requested", PropertyInfo(Variant::ARRAY, "choices")));
  ADD_SIGNAL(MethodInfo("mode_input_enabled_changed", PropertyInfo(Variant::BOOL, "enabled")));
  ADD_SIGNAL(MethodInfo("mode_entered", PropertyInfo(Variant::STRING, "mode_id"), PropertyInfo(Variant::STRING, "scene_id")));
  ADD_SIGNAL(MethodInfo("mode_exited", PropertyInfo(Variant::STRING, "current_mode_id"), PropertyInfo(Variant::STRING, "next_scene_id")));
}

void ScenarioRunner::_ready() {
  set_process(true);

  load_scenario_internal();
  if (!scenario_root_.is_empty()) {
    String start_id = dict_get_string(scenario_root_, "start_scene", "");
    if (!start_id.is_empty())
      load_scene_by_id(start_id);
  }
}

void ScenarioRunner::_process(double delta) {
  auto *clock = KernelClock::get_singleton();
  if (!clock) {
    godot::UtilityFunctions::push_error("[ScenarioRunner] KernelClock singleton missing");
    return;
  }
  clock->advance(delta);
  step_actions();
}

void ScenarioRunner::load_scenario() { load_scenario_internal(); }

bool ScenarioRunner::load_scenario_internal() {
  scenario_root_.clear();
  scenes_.clear();
  current_scene_id_ = "";
  if (scenario_path_.is_empty())
    return false;
  Ref<FileAccess> f = FileAccess::open(scenario_path_, FileAccess::READ);
  if (f.is_null())
    return false;
  Variant root;
  String err;

  if (scenario_path_.ends_with(".json")) {
    root = JSON::parse_string(f->get_as_text());
    if (root.get_type() == Variant::NIL) {
      godot::UtilityFunctions::print(
          String("[ScenarioRunner] Error: Failed to parse JSON: ") +
          scenario_path_);
      return false;
    }
  } else {
    if (!YamlLite::parse(f->get_as_text(), root, err)) {
      godot::UtilityFunctions::print(
          String("[ScenarioRunner] Error: Failed to parse YAML: ") + err);
      return false;
    }
  }

  scenario_root_ = as_dict(root);
  Dictionary raw_scenes = as_dict(scenario_root_.get("scenes", Dictionary()));

  // コンパイルフェーズ: 全てのシーンとアクションを事前に検証・インスタンス化する
  Array scene_keys = raw_scenes.keys();
  for (int i = 0; i < scene_keys.size(); i++) {
    String scene_id = scene_keys[i];
    Dictionary scene_dict = as_dict(raw_scenes[scene_id]);

    CompiledScene compiled;
    compiled.raw_dict = scene_dict;

    Array actions = as_array(scene_dict.get("on_enter", Array()));
    for (int j = 0; j < actions.size(); j++) {
      Ref<TaskBase> task = compile_action(actions[j]);
      if (task.is_null()) {
        godot::UtilityFunctions::push_error(
            String("[ScenarioRunner] Scene \"") + scene_id +
            "\" のアクションコンパイルに失敗しました。ロードを中止します。");
        scenes_.clear();
        return false;
      }
      compiled.tasks.append(task);
    }
    scenes_[scene_id] = compiled;
  }

  godot::UtilityFunctions::print(
      String("[ScenarioRunner] Loaded and compiled scenario from ") +
      scenario_path_ + ". Scenes count: " + String::num(scenes_.size()));
  return !scenes_.is_empty();
}

Ref<TaskBase> ScenarioRunner::compile_action(const Variant &action) {
  Dictionary d = as_dict(action);
  if (d.is_empty())
    return nullptr;

  String action_name = "";
  Dictionary payload;

  // 1. 形式の判別
  if (d.has("action")) {
    // 明示的形式: { action: "dialogue", text: "..." }
    action_name = d["action"];
    payload = d;
  } else if (d.size() == 1) {
    // 短縮形式: { dialogue: "..." } または { dialogue: { text: "..." } }
    action_name = d.keys()[0];
    Variant val = d[action_name];
    if (val.get_type() == Variant::DICTIONARY) {
      payload = val;
    } else {
      // { wait: 1.0 } -> { value: 1.0 } に正規化
      payload["value"] = val; 
    }
  }

  if (action_name.is_empty()) return nullptr;

  // 2. ActionRegistry 経由でのコンパイル (新形式 ABI v1)
  ActionRegistry *reg = ActionRegistry::get_singleton();
  if (reg && reg->has_action(action_name)) {
    // ダイアログ等は payload["text"] を期待する場合があるため、
    // 必要に応じて展開する (backward compatibility for normalization)
    if (action_name == "dialogue" && !payload.has("text") && !payload.has("text_key") && payload.has("value")) {
       payload["text"] = payload["value"];
    }

    // ABI v2.0: TaskSpec IR 作成
    TaskSpec spec_ir(action_name, payload);

    Ref<TaskBase> task = reg->compile_task(spec_ir);
    if (task.is_valid()) {
      task->set_runner(this);
      return task;
    }
  }

  // 3. ActionHandler フォールバック (Mystery 層カスタムアクション用)
  // ロード時にラッパータスクを生成し、CompiledScene.tasks に格納する。
  if (action_handlers_.has(action_name)) {
    Variant call_payload = payload.is_empty()
                               ? Variant(action_name)
                               : Variant(payload);
    Ref<InlineHandlerTask> task = memnew(InlineHandlerTask(
        action_handlers_[action_name], call_payload));
    task->set_runner(this);
    return task;
  }

  // 4. 未登録アクション: Fail-Fast
  UtilityFunctions::push_error(String("[ScenarioRunner] 未登録または未対応のアクション: \"") + action_name + "\". ActionRegistry に register_action() で登録してください。");
  return nullptr;
}

void ScenarioRunner::load_scene_by_id(const String &scene_id) {
  // シーン遷移時に SCOPE_SCENE のメモリを解放し、前シーンのローカル状態を破棄する
  if (auto *ws = WorldState::get_singleton()) {
    ws->clear_scope(WorldState::SCOPE_SCENE);
  }

  godot::UtilityFunctions::print(String("[ScenarioRunner] load_scene_by_id: ") +
                                 scene_id);
  if (!scenes_.has(scene_id)) {
    godot::UtilityFunctions::print(
        String("[ScenarioRunner] Error: Scene ID not found: ") + scene_id);
    return;
  }
  const CompiledScene &compiled = scenes_[scene_id];
  const Dictionary scene_dict = compiled.raw_dict;
  const String scene_path = dict_get_string(scene_dict, "scene_path", "");

  notify_mode_exit(current_mode_id_, scene_id);
  current_scene_id_ = scene_id;

  if (!scene_path.is_empty()) {
    Dictionary params;
    request_transition(scene_path, params);
  }

  notify_mode_enter(scene_id, scene_dict);
  
  godot::UtilityFunctions::print(
      String("[ScenarioRunner] starting compiled actions for ") + scene_id +
      ". Action count: " + String::num(compiled.tasks.size()));
  start_actions(compiled.tasks);
}

void ScenarioRunner::start_actions(const Array &actions) {
  if (actions.is_empty())
    return;
  pending_actions_ = actions;
  pending_action_index_ = 0;
  wait_remaining_sec_ = 0.0;
  is_executing_actions_ = true;
  set_mode_input_enabled(false);
}

void ScenarioRunner::step_actions() {
  if (!is_executing_actions_)
    return;

  if (waiting_for_transition_) {
    auto *clock = KernelClock::get_singleton();
    if (!clock) return;

    if (clock->now() - transition_start_time_ > 5.0) { // 5秒タイムアウト
      godot::UtilityFunctions::push_error("[ScenarioRunner] Scene transition timeout. complete_transition() was not called.");
      emit_signal("transition_failed", "timeout");
      waiting_for_transition_ = false;
      transition_start_time_ = 0.0;
      
      // abort_current_task() & advance_to_next_task()
      pending_action_index_++;
    } else {
      return;
    }
  }

  // consolidated execute loop: タスクの戻り値に基づいて進行を制御する
  while (pending_action_index_ < pending_actions_.size()) {
    Ref<TaskBase> task = pending_actions_[pending_action_index_];
    if (task.is_null()) {
      pending_action_index_++;
      continue;
    }

    TaskResult res = task->execute();

    if (res == TaskResult::Success) {
      pending_action_index_++;
      // 同一フレーム内で次のアクションに進む
      continue;
    } else if (res == TaskResult::Waiting) {
      // 入力待ち。現在のフレームの処理を終了。
      return;
    } else if (res == TaskResult::Yielded) {
      // 演出中など、1フレーム待機して次フレームで再開。
      return;
    } else if (res == TaskResult::Failed) {
      // 致命的エラー。実行を停止。
      is_executing_actions_ = false;
      godot::UtilityFunctions::push_error(
          "[ScenarioRunner] Task execution failed. Stopping scenario.");
      return;
    }
  }

  if (pending_action_index_ >= pending_actions_.size()) {
    is_executing_actions_ = false;
    set_mode_input_enabled(true);
  }
}

void ScenarioRunner::advance_dialogue() {
  if (waiting_for_dialogue_) {
    waiting_for_dialogue_ = false;
  }
}

void ScenarioRunner::submit_choice(int index) {
  if (waiting_for_choice_ && index >= 0 && index < pending_choice_actions_.size()) {
    waiting_for_choice_ = false;
    Dictionary choice = as_dict(pending_choice_actions_[index]);
    Array actions = as_array(choice.get("then", Array()));
    if (!actions.is_empty()) {
      inject_steps(actions);
    }
    pending_choice_actions_.clear();
  }
}

void ScenarioRunner::set_mode_input_enabled(bool enabled) {
  mode_input_enabled_ = enabled;
  emit_signal("mode_input_enabled_changed", enabled);
}

void ScenarioRunner::notify_mode_exit(const String &m, const String &n) {
  emit_signal("mode_exited", m, n);
}

void ScenarioRunner::notify_mode_enter(const String &s, const Dictionary &d) {
  current_mode_id_ = resolve_mode_id(s, d);
  emit_signal("mode_entered", current_mode_id_, s);
  set_mode_input_enabled(true);
}

String ScenarioRunner::resolve_mode_id(const String &s,
                                       const Dictionary &d) const {
  if (d.has("mode"))
    return d["mode"];
  return (s.find("confrontation") >= 0) ? "confrontation" : "investigation";
}

} // namespace karakuri
