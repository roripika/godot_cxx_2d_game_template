#include "scenario_runner.h"
#include "../action_registry.h"
#include "../logger/karakuri_logger.h"
#include "../logic/condition_evaluator.h"
#include "../services/flag_service.h"
#include "../services/save_service.h"
#include "../tasks/sequence_player.h"
#include "../yaml/yaml_lite.h"

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/circle_shape2d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/rectangle_shape2d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

namespace {
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
static CollisionShape2D *find_collision_shape(Area2D *area) {
  if (!area)
    return nullptr;
  for (int i = 0; i < area->get_child_count(); i++) {
    Node *c = area->get_child(i);
    if (CollisionShape2D *cs = Object::cast_to<CollisionShape2D>(c))
      return cs;
  }
  return nullptr;
}
} // namespace

ScenarioRunner::ScenarioRunner() { init_builtin_actions(); }
ScenarioRunner::~ScenarioRunner() {}

void ScenarioRunner::register_action(const String &kind,
                                     ActionHandler handler) {
  action_handlers_[kind] = handler;
}

void ScenarioRunner::set_scenario_path(const String &path) {
  scenario_path_ = path;
}
String ScenarioRunner::get_scenario_path() const { return scenario_path_; }

void ScenarioRunner::set_scene_container_path(const NodePath &path) {
  scene_container_path_ = path;
}
NodePath ScenarioRunner::get_scene_container_path() const {
  return scene_container_path_;
}

void ScenarioRunner::set_dialogue_ui_path(const NodePath &path) {
  dialogue_ui_path_ = path;
}
NodePath ScenarioRunner::get_dialogue_ui_path() const {
  return dialogue_ui_path_;
}

void ScenarioRunner::set_evidence_ui_path(const NodePath &path) {
  evidence_ui_path_ = path;
}
NodePath ScenarioRunner::get_evidence_ui_path() const {
  return evidence_ui_path_;
}

void ScenarioRunner::set_interaction_manager_path(const NodePath &path) {
  interaction_manager_path_ = path;
}
NodePath ScenarioRunner::get_interaction_manager_path() const {
  return interaction_manager_path_;
}

void ScenarioRunner::set_transition_manager_path(const NodePath &path) {
  transition_manager_path_ = path;
}
NodePath ScenarioRunner::get_transition_manager_path() const {
  return transition_manager_path_;
}

void ScenarioRunner::set_transition_rect_path(const NodePath &path) {
  transition_rect_path_ = path;
}
NodePath ScenarioRunner::get_transition_rect_path() const {
  return transition_rect_path_;
}

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

void ScenarioRunner::set_sequence_player_path(const NodePath &path) {
  sequence_player_path_ = path;
}

NodePath ScenarioRunner::get_sequence_player_path() const {
  return sequence_player_path_;
}

Node *ScenarioRunner::find_sequence_player() const {
  if (!sequence_player_path_.is_empty()) {
    return get_node_or_null(sequence_player_path_);
  }
  return nullptr;
}

void ScenarioRunner::complete_custom_action() {
  waiting_for_custom_action_ = false;
}

void ScenarioRunner::inject_steps(const Array &steps) {
  if (steps.is_empty())
    return;
  // pending_action_index_ はすでにインクリメント済みなので、現在位置に挿入する。
  Array before;
  for (int i = 0; i < pending_action_index_; ++i) {
    before.append(pending_actions_[i]);
  }
  Array after;
  for (int i = pending_action_index_; i < pending_actions_.size(); ++i) {
    after.append(pending_actions_[i]);
  }
  Array merged;
  for (int i = 0; i < before.size(); ++i)
    merged.append(before[i]);
  for (int i = 0; i < steps.size(); ++i)
    merged.append(steps[i]);
  for (int i = 0; i < after.size(); ++i)
    merged.append(after[i]);
  pending_actions_ = merged;
  // pending_action_index_ はそのまま。次の step_actions() で挿入分が実行される。
}

void ScenarioRunner::_bind_methods() {
  ClassDB::bind_method(D_METHOD("on_clicked_at", "pos"),
                       &ScenarioRunner::on_clicked_at);
  ClassDB::bind_method(D_METHOD("on_choice_selected", "index", "text"),
                       &ScenarioRunner::on_choice_selected);
  ClassDB::bind_method(D_METHOD("on_dialogue_finished"),
                       &ScenarioRunner::on_dialogue_finished);
  ClassDB::bind_method(D_METHOD("on_evidence_selected", "evidence_id"),
                       &ScenarioRunner::on_evidence_selected);
  ClassDB::bind_method(
      D_METHOD("on_transition_finished", "arg1", "arg2", "arg3"),
      &ScenarioRunner::on_transition_finished, DEFVAL(Variant()),
      DEFVAL(Variant()), DEFVAL(Variant()));

  // register_action (uses std::function) cannot be bound to ClassDB directly.
  // It is intended for C++ internal extension.

  ClassDB::bind_method(D_METHOD("load_scenario"),
                       &ScenarioRunner::load_scenario);
  ClassDB::bind_method(D_METHOD("load_scene_by_id", "scene_id"),
                       &ScenarioRunner::load_scene_by_id);
  ClassDB::bind_method(D_METHOD("execute_single_action", "action"),
                       &ScenarioRunner::execute_single_action);
  ClassDB::bind_method(D_METHOD("get_current_scene_id"),
                       &ScenarioRunner::get_current_scene_id);
  ClassDB::bind_method(D_METHOD("get_pending_action_index"),
                       &ScenarioRunner::get_pending_action_index);
  ClassDB::bind_method(D_METHOD("restore_to", "scene_id", "action_index"),
                       &ScenarioRunner::restore_to);
  ClassDB::bind_method(D_METHOD("is_running"), &ScenarioRunner::is_running);

  ClassDB::bind_method(D_METHOD("set_sequence_player_path", "path"),
                       &ScenarioRunner::set_sequence_player_path);
  ClassDB::bind_method(D_METHOD("get_sequence_player_path"),
                       &ScenarioRunner::get_sequence_player_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "sequence_player_path"),
               "set_sequence_player_path", "get_sequence_player_path");

  ClassDB::bind_method(D_METHOD("set_scenario_path", "path"),
                       &ScenarioRunner::set_scenario_path);
  ClassDB::bind_method(D_METHOD("get_scenario_path"),
                       &ScenarioRunner::get_scenario_path);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "scenario_path"),
               "set_scenario_path", "get_scenario_path");

  ClassDB::bind_method(D_METHOD("set_scene_container_path", "path"),
                       &ScenarioRunner::set_scene_container_path);
  ClassDB::bind_method(D_METHOD("get_scene_container_path"),
                       &ScenarioRunner::get_scene_container_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "scene_container_path"),
               "set_scene_container_path", "get_scene_container_path");

  ClassDB::bind_method(D_METHOD("set_dialogue_ui_path", "path"),
                       &ScenarioRunner::set_dialogue_ui_path);
  ClassDB::bind_method(D_METHOD("get_dialogue_ui_path"),
                       &ScenarioRunner::get_dialogue_ui_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "dialogue_ui_path"),
               "set_dialogue_ui_path", "get_dialogue_ui_path");

  ClassDB::bind_method(D_METHOD("set_evidence_ui_path", "path"),
                       &ScenarioRunner::set_evidence_ui_path);
  ClassDB::bind_method(D_METHOD("get_evidence_ui_path"),
                       &ScenarioRunner::get_evidence_ui_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "evidence_ui_path"),
               "set_evidence_ui_path", "get_evidence_ui_path");

  ClassDB::bind_method(D_METHOD("set_interaction_manager_path", "path"),
                       &ScenarioRunner::set_interaction_manager_path);
  ClassDB::bind_method(D_METHOD("get_interaction_manager_path"),
                       &ScenarioRunner::get_interaction_manager_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "interaction_manager_path"),
               "set_interaction_manager_path", "get_interaction_manager_path");

  ClassDB::bind_method(D_METHOD("set_transition_manager_path", "path"),
                       &ScenarioRunner::set_transition_manager_path);
  ClassDB::bind_method(D_METHOD("get_transition_manager_path"),
                       &ScenarioRunner::get_transition_manager_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "transition_manager_path"),
               "set_transition_manager_path", "get_transition_manager_path");

  ClassDB::bind_method(D_METHOD("set_transition_rect_path", "path"),
                       &ScenarioRunner::set_transition_rect_path);
  ClassDB::bind_method(D_METHOD("get_transition_rect_path"),
                       &ScenarioRunner::get_transition_rect_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "transition_rect_path"),
               "set_transition_rect_path", "get_transition_rect_path");
}

void ScenarioRunner::_ready() {
  set_process(true);
  scene_container_ = resolve_node_path(scene_container_path_);
  dialogue_ui_ = resolve_node_path(dialogue_ui_path_);
  evidence_ui_ = resolve_node_path(evidence_ui_path_);
  interaction_manager_ = resolve_node_path(interaction_manager_path_);
  transition_manager_ = resolve_node_path(transition_manager_path_);
  transition_rect_ = resolve_node_path(transition_rect_path_);

  if (interaction_manager_ && interaction_manager_->has_signal("clicked_at")) {
    interaction_manager_->connect("clicked_at",
                                  Callable(this, "on_clicked_at"));
  }
  if (dialogue_ui_) {
    if (dialogue_ui_->has_signal("choice_selected"))
      dialogue_ui_->connect("choice_selected",
                            Callable(this, "on_choice_selected"));
    if (dialogue_ui_->has_signal("dialogue_finished"))
      dialogue_ui_->connect("dialogue_finished",
                            Callable(this, "on_dialogue_finished"));
  }
  if (evidence_ui_ && evidence_ui_->has_signal("evidence_selected")) {
    evidence_ui_->connect("evidence_selected",
                          Callable(this, "on_evidence_selected"));
  }

  load_scenario_internal();
  if (!scenario_root_.is_empty()) {
    String start_id = dict_get_string(scenario_root_, "start_scene", "");
    if (!start_id.is_empty())
      load_scene_by_id(start_id);
  }
}

void ScenarioRunner::_process(double delta) {
  if (waiting_for_transition_ && transition_timeout_sec_ > 0.0f) {
    transition_timeout_sec_ -= (float)delta;
    if (transition_timeout_sec_ <= 0.0f) {
      waiting_for_transition_ = false;
      if (!transition_target_id_.is_empty())
        load_scene_by_id(transition_target_id_);
    }
  }
  step_actions(delta);
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
  scenes_ = as_dict(scenario_root_.get("scenes", Dictionary()));
  godot::UtilityFunctions::print(
      String("[ScenarioRunner] Loaded scenario from ") + scenario_path_ +
      ". Scenes count: " + String::num(scenes_.size()));
  return !scenes_.is_empty();
}

void ScenarioRunner::load_scene_by_id(const String &scene_id) {
  godot::UtilityFunctions::print(String("[ScenarioRunner] load_scene_by_id: ") +
                                 scene_id);
  if (!scenes_.has(scene_id)) {
    godot::UtilityFunctions::print(
        String("[ScenarioRunner] Error: Scene ID not found: ") + scene_id);
    return;
  }
  const Dictionary scene_dict = as_dict(scenes_[scene_id]);
  const String scene_path = dict_get_string(scene_dict, "scene_path", "");

  // can_rollback: true の場合、シーン開始前にスナップショットをスタックに積む
  // (アクション開始前の状態を保存するため、start_actions より前で行う)
  bool can_rollback = bool(scene_dict.get("can_rollback", false));
  if (can_rollback) {
    if (Node *sp_node = find_sequence_player()) {
      if (SequencePlayer *sp = Object::cast_to<SequencePlayer>(sp_node)) {
        sp->create_snapshot();
      }
    }
  }

  notify_mode_exit(current_mode_id_, scene_id);
  current_scene_id_ = scene_id;

  if (!scene_path.is_empty() && scene_container_) {
    for (int i = scene_container_->get_child_count() - 1; i >= 0; i--) {
      Node *c = scene_container_->get_child(i);
      if (c)
        c->queue_free();
    }
    Ref<PackedScene> packed = ResourceLoader::get_singleton()->load(scene_path);
    if (packed.is_valid()) {
      Node *inst = packed->instantiate();
      if (inst) {
        scene_container_->add_child(inst);
        current_scene_instance_ = inst;
      }
    }
  }

  if (dialogue_ui_) {
    if (dialogue_ui_->has_method("hide_dialogue"))
      dialogue_ui_->call("hide_dialogue");
    else
      dialogue_ui_->set("visible", false);
  }

  bind_scene_hotspots(scene_dict);
  notify_mode_enter(scene_id, scene_dict);
  Array actions = as_array(scene_dict.get("on_enter", Array()));
  godot::UtilityFunctions::print(
      String("[ScenarioRunner] starting actions for ") + scene_id +
      ". Action count: " + String::num(actions.size()));
  start_actions(actions);
}

void ScenarioRunner::bind_scene_hotspots(const Dictionary &scene_dict) {
  hotspot_bindings_.clear();
  if (!current_scene_instance_)
    return;
  const Dictionary hotspots = as_dict(scene_dict.get("hotspots", Dictionary()));
  const Array keys = hotspots.keys();
  for (int i = 0; i < keys.size(); i++) {
    String hs_id = keys[i];
    Dictionary hs_dict = as_dict(hotspots[hs_id]);
    String node_id = dict_get_string(hs_dict, "node_id", "");
    if (node_id.is_empty())
      continue;
    if (Node *n = current_scene_instance_->find_child(node_id, true, false)) {
      if (Area2D *area = Object::cast_to<Area2D>(n)) {
        bool visible = bool(hs_dict.get("visible", true));
        area->set_visible(visible);
        area->set_process_mode(visible ? PROCESS_MODE_INHERIT
                                       : PROCESS_MODE_DISABLED);

        HotspotBinding b;
        b.hotspot_id = hs_id;
        b.node_id = node_id;
        b.on_click_actions = as_array(hs_dict.get("on_click", Array()));

        Dictionary b_dict;
        b_dict["hotspot_id"] = b.hotspot_id;
        b_dict["node_id"] = b.node_id;
        b_dict["on_click"] = b.on_click_actions;
        hotspot_bindings_.append(b_dict);
      }
    }
  }
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

void ScenarioRunner::step_actions(double delta) {
  if (!is_executing_actions_)
    return;
  if (waiting_for_choice_ || waiting_for_dialogue_ || waiting_for_transition_ ||
      waiting_for_custom_action_)
    return;
  if (wait_remaining_sec_ > 0.0) {
    wait_remaining_sec_ -= delta;
    return;
  }

  while (pending_action_index_ < pending_actions_.size()) {
    if (execute_single_action(pending_actions_[pending_action_index_++]))
      break;
  }

  if (pending_action_index_ >= pending_actions_.size()) {
    if (!waiting_for_dialogue_ && !waiting_for_choice_ &&
        !waiting_for_transition_) {
      is_executing_actions_ = false;
      set_mode_input_enabled(true);
    }
  }
}

bool ScenarioRunner::execute_single_action(const Variant &action) {
  Dictionary d = as_dict(action);
  if (d.is_empty())
    return false;

  // ── 新形式: { "action": "add_evidence", "evidence_id": "knife", ... }
  // action キーが存在する場合は ActionRegistry 経由で動的ディスパッチする。
  // Core 層は Mystery 層の型を一切知らない。リフレクション (set) でパラメータを注入。
  if (d.has("action")) {
    const String action_name = d["action"];
    ActionRegistry *reg = ActionRegistry::get_singleton();
    if (reg == nullptr || !reg->has_action(action_name)) {
      UtilityFunctions::push_warning(
          String("[ScenarioRunner] ActionRegistry に未登録のアクション: \"") +
          action_name + "\"");
      return false;
    }

    Ref<TaskBase> task = reg->create_task(action_name);
    if (task.is_null()) {
      return false;
    }

    // JSON の残りキー ("action" を除く) をリフレクション (set) でタスクに注入する。
    // Core 層はタスクの具体的な型を知らなくてもプロパティを渡せる。
    Array keys = d.keys();
    for (int i = 0; i < keys.size(); i++) {
      const String k = keys[i];
      if (k == "action") continue;
      task->set(k, d[k]);
    }

    task->on_start();

    // 即時完了のタスク (TaskAddEvidence 等) は is_finished() == true → non-blocking
    // 非同期タスクは false → blocking (ScenarioPlayer が complete 待ちへ)
    const bool blocking = !task->is_finished();
    if (blocking) {
      waiting_for_custom_action_ = true;
    }
    return blocking;
  }

  // ── 旧形式: { "kind": value }  (builtin actions / lambda handlers)
  String kind = d.keys()[0];
  if (action_handlers_.has(kind)) {
    bool blocking = action_handlers_[kind](d[kind]);
    if (blocking && !waiting_for_dialogue_ && !waiting_for_choice_ &&
        !waiting_for_transition_) {
      waiting_for_custom_action_ = true;
    }
    return blocking;
  }
  return false;
}

void ScenarioRunner::init_builtin_actions() {
  register_action("wait", [this](const Variant &p) {
    wait_remaining_sec_ = (double)p;
    return true;
  });
  register_action("goto", [this](const Variant &p) {
    load_scene_by_id(String(p));
    return true;
  });
  register_action("dialogue", [this](const Variant &p) {
    Dictionary d = as_dict(p);
    godot::UtilityFunctions::print(
        String("[ScenarioRunner] dialogue action: speaker=") +
        dict_get_string(d, "speaker") + " text=" + dict_get_string(d, "text"));
    if (dialogue_ui_ && dialogue_ui_->has_method("show_message")) {
      waiting_for_dialogue_ = true;
      dialogue_ui_->call("show_message", dict_get_string(d, "speaker"),
                         dict_get_string(d, "text"));
      return true;
    } else {
      godot::UtilityFunctions::print(
          String("[ScenarioRunner] dialogue_ui_ is ") +
          (dialogue_ui_ ? "valid but missing show_message" : "null"));
    }
    return false;
  });

  // if — ConditionEvaluator 経由で条件分岐
  // YAML: { if: { condition: {...}, then: [...], else: [...] } }
  register_action("if", [this](const Variant &p) {
    Dictionary d = as_dict(p);
    Variant cond_v = d.has("condition") ? d["condition"] : Variant();
    bool result = false;
    if (cond_v.get_type() == Variant::DICTIONARY) {
      result = karakuri::ConditionEvaluator::evaluate(Dictionary(cond_v));
    } else if (cond_v.get_type() == Variant::BOOL) {
      result = bool(cond_v);
    }
    String branch_key = result ? "then" : "else";
    if (d.has(branch_key)) {
      Variant branch = d[branch_key];
      if (branch.get_type() == Variant::ARRAY) {
        inject_steps(Array(branch));
      }
    }
    return false; // Non-blocking
  });

  // set_flag — FlagService にフラグをセット
  // YAML: { set_flag: { name: "flag_name", value: true } }
  //       または { set_flag: "flag_name" }  (値は true になる)
  register_action("set_flag", [](const Variant &p) {
    auto *fs = karakuri::FlagService::get_singleton();
    if (!fs)
      return false;
    if (p.get_type() == Variant::STRING) {
      fs->set_flag(String(p), true);
    } else if (p.get_type() == Variant::DICTIONARY) {
      Dictionary d = p;
      String name = d.has("name") ? String(d["name"]) : String("");
      Variant value = d.has("value") ? d["value"] : Variant(true);
      if (!name.is_empty())
        fs->set_flag(name, value);
    }
    return false; // Non-blocking
  });
}

void ScenarioRunner::on_clicked_at(const Vector2 &pos) {
  if (is_executing_actions_ || !mode_input_enabled_)
    return;
  for (int i = 0; i < hotspot_bindings_.size(); i++) {
    Dictionary b = as_dict(hotspot_bindings_[i]);
    HotspotBinding hs;
    hs.node_id = dict_get_string(b, "node_id");
    hs.on_click_actions = as_array(b["on_click"]);
    if (hotspot_matches_click(hs, pos)) {
      trigger_hotspot(hs);
      return;
    }
  }
}

void ScenarioRunner::on_choice_selected(int index, const String &text) {
  waiting_for_choice_ = false;
}

void ScenarioRunner::on_dialogue_finished() { waiting_for_dialogue_ = false; }

void ScenarioRunner::on_evidence_selected(const String &evidence_id) {
  // Generic handler - can be overridden or extended by Mystery layer
}

void ScenarioRunner::on_transition_finished(const Variant &a1,
                                            const Variant &a2,
                                            const Variant &a3) {
  waiting_for_transition_ = false;
}

bool ScenarioRunner::hotspot_matches_click(const HotspotBinding &hs,
                                           const Vector2 &pos) const {
  if (!current_scene_instance_)
    return false;
  Node *n = current_scene_instance_->find_child(hs.node_id, true, false);
  Area2D *area = Object::cast_to<Area2D>(n);
  if (!area || !area->is_visible_in_tree())
    return false;

  CollisionShape2D *col = find_collision_shape(area);
  if (!col)
    return false;
  Ref<Shape2D> shape = col->get_shape();
  if (shape.is_null())
    return false;

  Transform2D gt = col->get_global_transform();
  Vector2 local_pos = gt.affine_inverse().xform(pos);

  if (CircleShape2D *c = Object::cast_to<CircleShape2D>(shape.ptr())) {
    return local_pos.length() <= c->get_radius();
  }
  if (RectangleShape2D *r = Object::cast_to<RectangleShape2D>(shape.ptr())) {
    Vector2 half = r->get_size() / 2.0;
    return local_pos.x >= -half.x && local_pos.x <= half.x &&
           local_pos.y >= -half.y && local_pos.y <= half.y;
  }
  return false;
}

void ScenarioRunner::trigger_hotspot(const HotspotBinding &hs) {
  if (!hs.on_click_actions.is_empty())
    start_actions(hs.on_click_actions);
}

void ScenarioRunner::set_mode_input_enabled(bool enabled) {
  mode_input_enabled_ = enabled;
  if (dialogue_ui_ && dialogue_ui_->has_method("set_mode_input_enabled"))
    dialogue_ui_->call("set_mode_input_enabled", enabled);
}

void ScenarioRunner::notify_mode_exit(const String &m, const String &n) {
  if (dialogue_ui_ && dialogue_ui_->has_method("on_mode_exit"))
    dialogue_ui_->call("on_mode_exit", m, n);
}

void ScenarioRunner::notify_mode_enter(const String &s, const Dictionary &d) {
  current_mode_id_ = resolve_mode_id(s, d);
  if (dialogue_ui_ && dialogue_ui_->has_method("on_mode_enter"))
    dialogue_ui_->call("on_mode_enter", current_mode_id_, s);
  set_mode_input_enabled(true);
}

String ScenarioRunner::resolve_mode_id(const String &s,
                                       const Dictionary &d) const {
  if (d.has("mode"))
    return d["mode"];
  return (s.find("confrontation") >= 0) ? "confrontation" : "investigation";
}

String ScenarioRunner::tr_key(const String &key) const {
  TranslationServer *ts = TranslationServer::get_singleton();
  if (!ts)
    return key;
  return (String)ts->translate(StringName(key));
}

Node *ScenarioRunner::resolve_node_path(const NodePath &p) const {
  return p.is_empty() ? nullptr : get_node_or_null(p);
}

Node *ScenarioRunner::get_adventure_state() const {
  if (get_tree() && get_tree()->get_root())
    return get_tree()->get_root()->get_node_or_null("AdventureGameState");
  return nullptr;
}

} // namespace karakuri
