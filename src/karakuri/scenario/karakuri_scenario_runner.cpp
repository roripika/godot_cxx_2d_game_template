#include "karakuri_scenario_runner.h"

/**
 * @file karakuri_scenario_runner.cpp
 * @brief See karakuri_scenario_runner.h
 */

#include "../yaml/karakuri_yaml_lite.h"

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/circle_shape2d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/rectangle_shape2d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {
namespace {

static Dictionary as_dict(const Variant &v) {
  if (v.get_type() == Variant::DICTIONARY) {
    return v;
  }
  return Dictionary();
}

static Array as_array(const Variant &v) {
  if (v.get_type() == Variant::ARRAY) {
    return v;
  }
  return Array();
}

static String dict_get_string(const Dictionary &d, const String &key,
                              const String &def = "") {
  if (!d.has(key)) {
    return def;
  }
  const Variant v = d[key];
  if (v.get_type() == Variant::STRING) {
    return v;
  }
  if (v.get_type() == Variant::STRING_NAME) {
    return String(v);
  }
  return String(v);
}

static bool dict_get_bool(const Dictionary &d, const String &key, bool def) {
  if (!d.has(key)) {
    return def;
  }
  const Variant v = d[key];
  if (v.get_type() == Variant::BOOL) {
    return bool(v);
  }
  return def;
}

static double dict_get_float(const Dictionary &d, const String &key,
                             double def) {
  if (!d.has(key)) {
    return def;
  }
  const Variant v = d[key];
  if (v.get_type() == Variant::FLOAT || v.get_type() == Variant::INT) {
    return double(v);
  }
  return def;
}

static CollisionShape2D *find_collision_shape(Area2D *area) {
  if (!area) {
    return nullptr;
  }
  for (int i = 0; i < area->get_child_count(); i++) {
    Node *c = area->get_child(i);
    CollisionShape2D *cs = Object::cast_to<CollisionShape2D>(c);
    if (cs) {
      return cs;
    }
  }
  return nullptr;
}

} // namespace

KarakuriScenarioRunner::KarakuriScenarioRunner() {}

KarakuriScenarioRunner::~KarakuriScenarioRunner() {}

void KarakuriScenarioRunner::set_scenario_path(const String &path) {
  scenario_path_ = path;
}

String KarakuriScenarioRunner::get_scenario_path() const {
  return scenario_path_;
}

void KarakuriScenarioRunner::set_scene_container_path(const NodePath &path) {
  scene_container_path_ = path;
}

NodePath KarakuriScenarioRunner::get_scene_container_path() const {
  return scene_container_path_;
}

void KarakuriScenarioRunner::set_dialogue_ui_path(const NodePath &path) {
  dialogue_ui_path_ = path;
}

NodePath KarakuriScenarioRunner::get_dialogue_ui_path() const {
  return dialogue_ui_path_;
}

void KarakuriScenarioRunner::set_evidence_ui_path(const NodePath &path) {
  evidence_ui_path_ = path;
}

NodePath KarakuriScenarioRunner::get_evidence_ui_path() const {
  return evidence_ui_path_;
}

void KarakuriScenarioRunner::set_interaction_manager_path(
    const NodePath &path) {
  interaction_manager_path_ = path;
}

NodePath KarakuriScenarioRunner::get_interaction_manager_path() const {
  return interaction_manager_path_;
}

void KarakuriScenarioRunner::set_testimony_system_path(const NodePath &path) {
  testimony_system_path_ = path;
}

NodePath KarakuriScenarioRunner::get_testimony_system_path() const {
  return testimony_system_path_;
}

void KarakuriScenarioRunner::_bind_methods() {
  // Signal handlers (must be bound to be callable through Callable connections).
  ClassDB::bind_method(D_METHOD("on_clicked_at", "pos"),
                       &KarakuriScenarioRunner::on_clicked_at);
  ClassDB::bind_method(D_METHOD("on_choice_selected", "index", "text"),
                       &KarakuriScenarioRunner::on_choice_selected);
  ClassDB::bind_method(D_METHOD("on_dialogue_finished"),
                       &KarakuriScenarioRunner::on_dialogue_finished);
  ClassDB::bind_method(D_METHOD("on_testimony_complete", "success"),
                       &KarakuriScenarioRunner::on_testimony_complete);
  ClassDB::bind_method(D_METHOD("on_testimony_next_requested"),
                       &KarakuriScenarioRunner::on_testimony_next_requested);
  ClassDB::bind_method(D_METHOD("on_testimony_shake_requested"),
                       &KarakuriScenarioRunner::on_testimony_shake_requested);
  ClassDB::bind_method(D_METHOD("on_testimony_present_requested"),
                       &KarakuriScenarioRunner::on_testimony_present_requested);
  ClassDB::bind_method(D_METHOD("on_evidence_selected", "evidence_id"),
                       &KarakuriScenarioRunner::on_evidence_selected);

  ClassDB::bind_method(D_METHOD("set_scenario_path", "path"),
                       &KarakuriScenarioRunner::set_scenario_path);
  ClassDB::bind_method(D_METHOD("get_scenario_path"),
                       &KarakuriScenarioRunner::get_scenario_path);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "scenario_path"),
               "set_scenario_path", "get_scenario_path");

  ClassDB::bind_method(D_METHOD("set_scene_container_path", "path"),
                       &KarakuriScenarioRunner::set_scene_container_path);
  ClassDB::bind_method(D_METHOD("get_scene_container_path"),
                       &KarakuriScenarioRunner::get_scene_container_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "scene_container_path"),
               "set_scene_container_path", "get_scene_container_path");

  ClassDB::bind_method(D_METHOD("set_dialogue_ui_path", "path"),
                       &KarakuriScenarioRunner::set_dialogue_ui_path);
  ClassDB::bind_method(D_METHOD("get_dialogue_ui_path"),
                       &KarakuriScenarioRunner::get_dialogue_ui_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "dialogue_ui_path"),
               "set_dialogue_ui_path", "get_dialogue_ui_path");

  ClassDB::bind_method(D_METHOD("set_evidence_ui_path", "path"),
                       &KarakuriScenarioRunner::set_evidence_ui_path);
  ClassDB::bind_method(D_METHOD("get_evidence_ui_path"),
                       &KarakuriScenarioRunner::get_evidence_ui_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "evidence_ui_path"),
               "set_evidence_ui_path", "get_evidence_ui_path");

  ClassDB::bind_method(D_METHOD("set_interaction_manager_path", "path"),
                       &KarakuriScenarioRunner::set_interaction_manager_path);
  ClassDB::bind_method(D_METHOD("get_interaction_manager_path"),
                       &KarakuriScenarioRunner::get_interaction_manager_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "interaction_manager_path"),
               "set_interaction_manager_path", "get_interaction_manager_path");

  ClassDB::bind_method(D_METHOD("set_testimony_system_path", "path"),
                       &KarakuriScenarioRunner::set_testimony_system_path);
  ClassDB::bind_method(D_METHOD("get_testimony_system_path"),
                       &KarakuriScenarioRunner::get_testimony_system_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "testimony_system_path"),
               "set_testimony_system_path", "get_testimony_system_path");
}

void KarakuriScenarioRunner::_ready() {
  // Ensure _process runs for the action runner.
  set_process(true);

  scene_container_ = resolve_node_path(scene_container_path_);
  dialogue_ui_ = resolve_node_path(dialogue_ui_path_);
  evidence_ui_ = resolve_node_path(evidence_ui_path_);
  interaction_manager_ = resolve_node_path(interaction_manager_path_);
  testimony_system_ = resolve_node_path(testimony_system_path_);

  if (!scene_container_) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: scene_container not found: ",
        String(scene_container_path_));
    return;
  }

  if (!interaction_manager_ ||
      !interaction_manager_->has_signal("clicked_at")) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: InteractionManager missing or no clicked_at "
        "signal at: ",
        String(interaction_manager_path_));
    return;
  }

  const Callable cb(this, "on_clicked_at");
  const Error err = interaction_manager_->connect("clicked_at", cb);
  if (err != OK && err != ERR_ALREADY_EXISTS) {
    UtilityFunctions::push_warning(
        "KarakuriScenarioRunner: failed to connect clicked_at: ", err);
  }

  if (dialogue_ui_ && dialogue_ui_->has_signal("choice_selected")) {
    const Callable cb2(this, "on_choice_selected");
    const Error err2 = dialogue_ui_->connect("choice_selected", cb2);
    if (err2 != OK && err2 != ERR_ALREADY_EXISTS) {
      UtilityFunctions::push_warning(
          "KarakuriScenarioRunner: failed to connect choice_selected: ", err2);
    }
  }

  if (dialogue_ui_ && dialogue_ui_->has_signal("dialogue_finished")) {
    const Callable cbdf(this, "on_dialogue_finished");
    const Error errdf = dialogue_ui_->connect("dialogue_finished", cbdf);
    if (errdf != OK && errdf != ERR_ALREADY_EXISTS) {
      UtilityFunctions::push_warning(
          "KarakuriScenarioRunner: failed to connect dialogue_finished: ",
          errdf);
    }
  }

  if (testimony_system_) {
    if (testimony_system_->has_signal("next_requested")) {
      const Callable cb_next(this, "on_testimony_next_requested");
      const Error err_next = testimony_system_->connect("next_requested", cb_next);
      if (err_next != OK && err_next != ERR_ALREADY_EXISTS) {
        UtilityFunctions::push_warning(
            "KarakuriScenarioRunner: failed to connect next_requested: ",
            err_next);
      }
    }
    if (testimony_system_->has_signal("shake_requested")) {
      const Callable cb_shake(this, "on_testimony_shake_requested");
      const Error err_shake =
          testimony_system_->connect("shake_requested", cb_shake);
      if (err_shake != OK && err_shake != ERR_ALREADY_EXISTS) {
        UtilityFunctions::push_warning(
            "KarakuriScenarioRunner: failed to connect shake_requested: ",
            err_shake);
      }
    }
    if (testimony_system_->has_signal("present_requested")) {
      const Callable cb_present(this, "on_testimony_present_requested");
      const Error err_present =
          testimony_system_->connect("present_requested", cb_present);
      if (err_present != OK && err_present != ERR_ALREADY_EXISTS) {
        UtilityFunctions::push_warning(
            "KarakuriScenarioRunner: failed to connect present_requested: ",
            err_present);
      }
    }

    // Backward compatibility for older testimony scripts.
    if (testimony_system_->has_signal("all_rounds_complete")) {
      const Callable cb_complete(this, "on_testimony_complete");
      const Error err_complete =
          testimony_system_->connect("all_rounds_complete", cb_complete);
      if (err_complete != OK && err_complete != ERR_ALREADY_EXISTS) {
        UtilityFunctions::push_warning(
            "KarakuriScenarioRunner: failed to connect all_rounds_complete: ",
            err_complete);
      }
    }
  }

  if (evidence_ui_ && evidence_ui_->has_signal("evidence_selected")) {
    const Callable cb_es(this, "on_evidence_selected");
    const Error err_es = evidence_ui_->connect("evidence_selected", cb_es);
    if (err_es != OK && err_es != ERR_ALREADY_EXISTS) {
      UtilityFunctions::push_warning(
          "KarakuriScenarioRunner: failed to connect evidence_selected: ",
          err_es);
    }
  }

  if (!load_scenario()) {
    return;
  }

  const String start_id = dict_get_string(scenario_root_, "start_scene", "");
  if (start_id.is_empty()) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: scenario missing start_scene");
    return;
  }

  load_scene_by_id(start_id);
}

void KarakuriScenarioRunner::_process(double delta) {
  step_actions(delta);
}

bool KarakuriScenarioRunner::load_scenario() {
  scenario_root_.clear();
  scenes_.clear();
  current_scene_id_ = "";

  Ref<FileAccess> f = FileAccess::open(scenario_path_, FileAccess::READ);
  if (f.is_null()) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: failed to open scenario YAML: ",
        scenario_path_);
    return false;
  }

  const String yaml_text = f->get_as_text();
  Variant root;
  String err;
  if (!KarakuriYamlLite::parse(yaml_text, root, err)) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: YAML parse error: ", err);
    return false;
  }

  scenario_root_ = as_dict(root);
  if (scenario_root_.is_empty()) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: scenario root is empty or not a map");
    return false;
  }

  scenes_ = as_dict(scenario_root_.get("scenes", Dictionary()));
  if (scenes_.is_empty()) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: scenario missing scenes map");
    return false;
  }

  return true;
}

bool KarakuriScenarioRunner::load_scene_by_id(const String &scene_id) {
  if (!scenes_.has(scene_id)) {
    UtilityFunctions::push_error("KarakuriScenarioRunner: unknown scene id: ",
                                 scene_id);
    return false;
  }

  const Dictionary scene_dict = as_dict(scenes_[scene_id]);
  const String scene_path = dict_get_string(scene_dict, "scene_path", "");
  if (scene_path.is_empty()) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: scene missing scene_path: ", scene_id);
    return false;
  }

  notify_mode_exit(scene_id);

  // Replace current base scene instance.
  for (int i = scene_container_->get_child_count() - 1; i >= 0; i--) {
    Node *c = scene_container_->get_child(i);
    if (c) {
      c->queue_free();
    }
  }
  current_scene_instance_ = nullptr;
  hotspot_bindings_.clear();

  Ref<PackedScene> packed =
      ResourceLoader::get_singleton()->load(scene_path);
  if (packed.is_null()) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: failed to load PackedScene: ", scene_path);
    return false;
  }

  Node *inst = packed->instantiate();
  if (!inst) {
    UtilityFunctions::push_error(
        "KarakuriScenarioRunner: failed to instantiate: ", scene_path);
    return false;
  }
  scene_container_->add_child(inst);

  current_scene_id_ = scene_id;
  current_scene_instance_ = inst;

  // Confrontation UI should not persist across scenes unless started by actions.
  if (testimony_system_) {
    if (testimony_system_->has_method("hide_panel")) {
      testimony_system_->call("hide_panel");
    } else {
      testimony_system_->set("visible", false);
    }
  }
  testimony_active_ = false;
  waiting_for_testimony_ = false;
  waiting_for_evidence_selection_ = false;

  bind_scene_hotspots(scene_dict);
  notify_mode_enter(scene_id, scene_dict);

  // Scene on_enter actions.
  const Array on_enter = as_array(scene_dict.get("on_enter", Array()));
  if (!on_enter.is_empty()) {
    start_actions(on_enter);
  }

  return true;
}

void KarakuriScenarioRunner::bind_scene_hotspots(const Dictionary &scene_dict) {
  hotspot_bindings_.clear();
  if (!current_scene_instance_) {
    return;
  }

  const Dictionary hotspots =
      as_dict(scene_dict.get("hotspots", Dictionary()));
  if (hotspots.is_empty()) {
    return;
  }

  const Array keys = hotspots.keys();
  Array bindings;
  for (int i = 0; i < keys.size(); i++) {
    const String hs_id = String(keys[i]);
    const Dictionary hs_dict = as_dict(hotspots[hs_id]);
    const String node_id = dict_get_string(hs_dict, "node_id", "");
    if (node_id.is_empty()) {
      continue;
    }
    // Do not filter by "owned" here: at runtime, PackedScene instances can
    // have null owners, and we still want to bind hotspots by name.
    Node *area_node =
        current_scene_instance_->find_child(node_id, true, false);
    Area2D *area = Object::cast_to<Area2D>(area_node);
    if (!area) {
      continue;
    }

    Dictionary b;
    b["hotspot_id"] = hs_id;
    b["node_id"] = node_id;
    b["on_click"] = as_array(hs_dict.get("on_click", Array()));
    bindings.append(b);
  }
  hotspot_bindings_ = bindings;
}

void KarakuriScenarioRunner::start_actions(const Array &actions) {
  pending_actions_ = actions;
  pending_action_index_ = 0;
  wait_remaining_sec_ = 0.0;
  is_executing_actions_ = true;
  waiting_for_choice_ = false;
  waiting_for_dialogue_ = false;
  waiting_for_testimony_ = false;
  set_mode_input_enabled(false);
}

void KarakuriScenarioRunner::step_actions(double delta) {
  if (!is_executing_actions_) {
    return;
  }
  if (waiting_for_choice_ || waiting_for_dialogue_ || waiting_for_testimony_) {
    return;
  }
  if (wait_remaining_sec_ > 0.0) {
    wait_remaining_sec_ -= delta;
    return;
  }
  if (pending_action_index_ >= pending_actions_.size()) {
    is_executing_actions_ = false;
    set_mode_input_enabled(true);
    return;
  }

  const Variant action = pending_actions_[pending_action_index_];
  pending_action_index_++;
  execute_single_action(action);
}

bool KarakuriScenarioRunner::execute_single_action(const Variant &action) {
  // Action is expected to be a Dictionary with a single key.
  const Dictionary d = as_dict(action);
  if (d.is_empty()) {
    return false;
  }
  const Array keys = d.keys();
  if (keys.is_empty()) {
    return false;
  }

  const String kind = String(keys[0]);
  const Variant payload_v = d[kind];

  if (kind == "dialogue") {
    const Dictionary payload = as_dict(payload_v);
    const String speaker_key = dict_get_string(payload, "speaker_key", "");
    const String speaker = speaker_key.is_empty()
                               ? dict_get_string(payload, "speaker", "System")
                               : tr_key(speaker_key);
    const String text_key = dict_get_string(payload, "text_key", "");
    const String text = text_key.is_empty()
                            ? dict_get_string(payload, "text", "")
                            : tr_key(text_key);

    if (dialogue_ui_ && dialogue_ui_->has_method("show_message")) {
      // Set this before calling into GDScript to avoid missing an immediate
      // synchronous `dialogue_finished` emission (e.g. typing_speed <= 0).
      waiting_for_dialogue_ = dialogue_ui_->has_signal("dialogue_finished");
      if (dialogue_ui_->has_method("show_message_with_keys")) {
        dialogue_ui_->call("show_message_with_keys", speaker_key, speaker,
                           text_key, text);
      } else {
        dialogue_ui_->call("show_message", speaker, text);
      }
    } else {
      UtilityFunctions::print("[Dialogue] ", speaker, ": ", text);
    }
    return true;
  }

  if (kind == "set_flag") {
    const Dictionary payload = as_dict(payload_v);
    const String flag = dict_get_string(payload, "key", "");
    const bool value = dict_get_bool(payload, "value", true);
    if (flag.is_empty()) {
      return false;
    }
    Node *gs = get_adventure_state();
    if (gs && gs->has_method("set_flag")) {
      gs->call("set_flag", flag, value);
    }
    return true;
  }

  if (kind == "give_evidence" || kind == "give_item") {
    const String item_id = String(payload_v);
    if (item_id.is_empty()) {
      return false;
    }
    Node *gs = get_adventure_state();
    if (gs && gs->has_method("add_item")) {
      gs->call("add_item", item_id);
    }
    if (evidence_ui_ && evidence_ui_->has_method("add_evidence")) {
      evidence_ui_->call("add_evidence", item_id);
      if (evidence_ui_->has_method("show_inventory")) {
        evidence_ui_->call("show_inventory");
      }
    }
    return true;
  }

  if (kind == "wait") {
    wait_remaining_sec_ = dict_get_float(d, kind, 0.0);
    if (wait_remaining_sec_ <= 0.0) {
      // If payload is scalar, support it too.
      if (payload_v.get_type() == Variant::FLOAT ||
          payload_v.get_type() == Variant::INT) {
        wait_remaining_sec_ = double(payload_v);
      }
    }
    return true;
  }

  if (kind == "goto") {
    const String next_id = String(payload_v);
    if (next_id.is_empty()) {
      return false;
    }
    load_scene_by_id(next_id);
    return true;
  }

  if (kind == "if_flag") {
    const Dictionary payload = as_dict(payload_v);
    const String key = dict_get_string(payload, "key", "");
    const bool expected = dict_get_bool(payload, "value", true);
    const Array then_actions = as_array(payload.get("then", Array()));
    const Array else_actions = as_array(payload.get("else", Array()));

    bool actual = false;
    Node *gs = get_adventure_state();
    if (gs && gs->has_method("get_flag") && !key.is_empty()) {
      actual = bool(gs->call("get_flag", key, false));
    }

    const Array chosen = (actual == expected) ? then_actions : else_actions;
    for (int i = chosen.size() - 1; i >= 0; i--) {
      pending_actions_.insert(pending_action_index_, chosen[i]);
    }
    return true;
  }

  if (kind == "if_has_items") {
    const Dictionary payload = as_dict(payload_v);
    const Array items = as_array(payload.get("items", Array()));
    const Array then_actions = as_array(payload.get("then", Array()));
    const Array else_actions = as_array(payload.get("else", Array()));

    bool ok = true;
    Node *gs = get_adventure_state();
    if (!gs || !gs->has_method("has_item")) {
      ok = false;
    }
    for (int i = 0; ok && i < items.size(); i++) {
      const String item = String(items[i]);
      if (item.is_empty()) {
        continue;
      }
      if (!bool(gs->call("has_item", item))) {
        ok = false;
        break;
      }
    }

    const Array chosen = ok ? then_actions : else_actions;
    for (int i = chosen.size() - 1; i >= 0; i--) {
      pending_actions_.insert(pending_action_index_, chosen[i]);
    }
    return true;
  }

  if (kind == "choice") {
    if (!dialogue_ui_ || !dialogue_ui_->has_method("show_choices") ||
        !dialogue_ui_->has_signal("choice_selected")) {
      UtilityFunctions::push_error(
          "KarakuriScenarioRunner: choice requires DialogueUIAdvanced with "
          "show_choices + choice_selected");
      return false;
    }

    const Dictionary payload = as_dict(payload_v);
    const Array choices = as_array(payload.get("choices", Array()));
    if (choices.is_empty()) {
      return false;
    }

    Array choice_texts;
    Array choice_defs;
    Array choice_actions; // Array<Array>
    for (int i = 0; i < choices.size(); i++) {
      Dictionary c = as_dict(choices[i]);
      if (c.has("option")) {
        c = as_dict(c["option"]);
      }
      const String key = dict_get_string(c, "text_key", "");
      const String txt =
          key.is_empty() ? dict_get_string(c, "text", "") : tr_key(key);
      choice_texts.append(txt);
      Dictionary def;
      def["text_key"] = key;
      def["text"] = dict_get_string(c, "text", txt);
      choice_defs.append(def);
      choice_actions.append(as_array(c.get("actions", Array())));
    }

    pending_choice_actions_ = choice_actions;
    waiting_for_choice_ = true;
    set_mode_input_enabled(true);

    // Fire-and-forget; show_choices itself waits for the signal internally.
    if (dialogue_ui_->has_method("show_choices_with_defs")) {
      dialogue_ui_->call("show_choices_with_defs", choice_defs);
    } else {
      dialogue_ui_->call("show_choices", choice_texts);
    }
    return true;
  }

  if (kind == "take_damage") {
    int amount = 1;
    if (payload_v.get_type() == Variant::INT) {
      amount = int(payload_v);
    } else if (payload_v.get_type() == Variant::DICTIONARY) {
      amount = int(as_dict(payload_v).get("amount", 1));
    }
    if (amount < 1) {
      amount = 1;
    }
    Node *gs = get_adventure_state();
    if (gs && gs->has_method("take_damage")) {
      for (int i = 0; i < amount; i++) {
        gs->call("take_damage");
      }
    }
    return true;
  }

  if (kind == "reset_game") {
    Node *gs = get_adventure_state();
    if (gs && gs->has_method("reset_game")) {
      gs->call("reset_game");
    }
    return true;
  }

  if (kind == "if_health_ge") {
    const Dictionary payload = as_dict(payload_v);
    const int threshold = int(payload.get("value", 0));
    const Array then_actions = as_array(payload.get("then", Array()));
    const Array else_actions = as_array(payload.get("else", Array()));

    int hp = 0;
    Node *gs = get_adventure_state();
    if (gs && gs->has_method("get_health")) {
      hp = int(gs->call("get_health"));
    }

    const Array chosen = (hp >= threshold) ? then_actions : else_actions;
    for (int i = chosen.size() - 1; i >= 0; i--) {
      pending_actions_.insert(pending_action_index_, chosen[i]);
    }
    return true;
  }

  if (kind == "if_health_leq") {
    const Dictionary payload = as_dict(payload_v);
    const int threshold = int(payload.get("value", 0));
    const Array then_actions = as_array(payload.get("then", Array()));
    const Array else_actions = as_array(payload.get("else", Array()));

    int hp = 0;
    Node *gs = get_adventure_state();
    if (gs && gs->has_method("get_health")) {
      hp = int(gs->call("get_health"));
    }

    const Array chosen = (hp <= threshold) ? then_actions : else_actions;
    for (int i = chosen.size() - 1; i >= 0; i--) {
      pending_actions_.insert(pending_action_index_, chosen[i]);
    }
    return true;
  }

  if (kind == "testimony") {
    if (!testimony_system_) {
      UtilityFunctions::push_error(
          "KarakuriScenarioRunner: testimony requires TestimonySystem node");
      return false;
    }

    const Dictionary payload = as_dict(payload_v);
    const Array testimonies = as_array(payload.get("testimonies", Array()));
    if (testimonies.is_empty()) {
      UtilityFunctions::push_error(
          "KarakuriScenarioRunner: testimony action has empty testimonies");
      return false;
    }
    pending_testimony_success_actions_ =
        as_array(payload.get("on_success", Array()));
    pending_testimony_failure_actions_ =
        as_array(payload.get("on_failure", Array()));

    testimony_max_rounds_ = int(payload.get("max_rounds", 1));
    if (testimony_max_rounds_ < 1) {
      testimony_max_rounds_ = 1;
    }
    testimony_round_ = 0;
    testimony_index_ = 0;
    testimony_active_ = true;
    waiting_for_testimony_ = true;
    waiting_for_evidence_selection_ = false;
    testimony_lines_.clear();

    for (int i = 0; i < testimonies.size(); i++) {
      Dictionary t = as_dict(testimonies[i]);
      if (t.has("line")) {
        t = as_dict(t["line"]);
      }
      Dictionary line;
      line["speaker_key"] = dict_get_string(t, "speaker_key", "");
      line["speaker_text"] = dict_get_string(t, "speaker", "Witness");
      line["text_key"] = dict_get_string(t, "text_key", "");
      line["text_text"] = dict_get_string(t, "text", "");
      line["evidence"] = dict_get_string(t, "evidence", "");
      const String shake_key = dict_get_string(t, "shake_key", "");
      line["shake_key"] = shake_key;
      line["shake_text"] = dict_get_string(t, "shake", "");
      line["solved"] = false;
      testimony_lines_.append(line);
    }

    set_mode_input_enabled(true);
    if (testimony_system_->has_method("show_panel")) {
      testimony_system_->call("show_panel");
    } else {
      testimony_system_->set("visible", true);
    }
    show_current_testimony_line();
    return true;
  }

  if (kind == "change_root_scene") {
    const String path = String(payload_v);
    if (path.is_empty() || !get_tree()) {
      return false;
    }
    get_tree()->change_scene_to_file(path);
    return true;
  }

  UtilityFunctions::push_warning("KarakuriScenarioRunner: unknown action: ",
                                 kind);
  return false;
}

void KarakuriScenarioRunner::on_clicked_at(const Vector2 &pos) {
  if (is_executing_actions_ || !mode_input_enabled_) {
    // Prevent accidental re-entry while scripted actions are running.
    return;
  }
  for (int i = 0; i < hotspot_bindings_.size(); i++) {
    const Dictionary b = as_dict(hotspot_bindings_[i]);
    if (b.is_empty()) {
      continue;
    }
    HotspotBinding hs;
    hs.hotspot_id = dict_get_string(b, "hotspot_id", "");
    hs.node_id = dict_get_string(b, "node_id", "");
    hs.on_click_actions = as_array(b.get("on_click", Array()));

    if (hotspot_matches_click(hs, pos)) {
      trigger_hotspot(hs);
      return;
    }
  }
}

void KarakuriScenarioRunner::on_choice_selected(int index, const String &text) {
  (void)text;
  if (!waiting_for_choice_) {
    return;
  }
  waiting_for_choice_ = false;
  set_mode_input_enabled(false);

  if (index < 0 || index >= pending_choice_actions_.size()) {
    return;
  }
  const Array chosen = as_array(pending_choice_actions_[index]);
  for (int i = chosen.size() - 1; i >= 0; i--) {
    pending_actions_.insert(pending_action_index_, chosen[i]);
  }
}

void KarakuriScenarioRunner::on_dialogue_finished() {
  if (!waiting_for_dialogue_) {
    return;
  }
  waiting_for_dialogue_ = false;
}

void KarakuriScenarioRunner::on_testimony_complete(bool success) {
  complete_testimony(success);
}

void KarakuriScenarioRunner::on_testimony_next_requested() {
  if (!testimony_active_ || waiting_for_evidence_selection_) {
    return;
  }
  testimony_index_++;
  if (testimony_index_ >= testimony_lines_.size()) {
    if (are_all_testimony_contradictions_solved()) {
      complete_testimony(true);
      return;
    }
    testimony_round_++;
    if (dialogue_ui_ && dialogue_ui_->has_method("show_message_with_keys")) {
      dialogue_ui_->call("show_message_with_keys", "speaker.system", "System",
                         "testimony_incomplete",
                         tr_key("testimony_incomplete"));
    } else if (dialogue_ui_ && dialogue_ui_->has_method("show_message")) {
      dialogue_ui_->call("show_message", "System", tr_key("testimony_incomplete"));
    }

    if (testimony_round_ >= testimony_max_rounds_) {
      complete_testimony(false);
      return;
    }
    testimony_index_ = 0;
  }
  show_current_testimony_line();
}

void KarakuriScenarioRunner::on_testimony_shake_requested() {
  if (!testimony_active_ || testimony_index_ < 0 ||
      testimony_index_ >= testimony_lines_.size()) {
    return;
  }
  const Dictionary line = as_dict(testimony_lines_[testimony_index_]);
  const String speaker_key = dict_get_string(line, "speaker_key", "");
  const String speaker = speaker_key.is_empty()
                             ? dict_get_string(line, "speaker_text", "Witness")
                             : tr_key(speaker_key);
  const String shake_key = dict_get_string(line, "shake_key", "");
  const String shake = shake_key.is_empty()
                           ? dict_get_string(line, "shake_text", "")
                           : tr_key(shake_key);
  if (shake.is_empty()) {
    return;
  }
  if (dialogue_ui_ && dialogue_ui_->has_method("show_message_with_keys")) {
    dialogue_ui_->call("show_message_with_keys", speaker_key, speaker, shake_key,
                       shake);
    return;
  }
  if (dialogue_ui_ && dialogue_ui_->has_method("show_message")) {
    dialogue_ui_->call("show_message", speaker, shake);
  }
}

void KarakuriScenarioRunner::on_testimony_present_requested() {
  if (!testimony_active_ || waiting_for_evidence_selection_ || !evidence_ui_) {
    return;
  }
  waiting_for_evidence_selection_ = true;
  if (testimony_system_ && testimony_system_->has_method("set_actions_enabled")) {
    testimony_system_->call("set_actions_enabled", false);
  }
  if (evidence_ui_->has_method("show_inventory")) {
    evidence_ui_->call("show_inventory");
  } else {
    evidence_ui_->set("visible", true);
  }
}

void KarakuriScenarioRunner::on_evidence_selected(const String &evidence_id) {
  if (!testimony_active_ || !waiting_for_evidence_selection_) {
    return;
  }
  waiting_for_evidence_selection_ = false;
  if (evidence_ui_ && evidence_ui_->has_method("hide_inventory")) {
    evidence_ui_->call("hide_inventory");
  }
  if (testimony_system_ && testimony_system_->has_method("set_actions_enabled")) {
    testimony_system_->call("set_actions_enabled", true);
  }

  if (testimony_index_ < 0 || testimony_index_ >= testimony_lines_.size()) {
    return;
  }

  Dictionary line = as_dict(testimony_lines_[testimony_index_]);
  const String expected = dict_get_string(line, "evidence", "");
  const bool correct = !expected.is_empty() && expected == evidence_id;

  if (correct) {
    line["solved"] = true;
    testimony_lines_[testimony_index_] = line;
    if (dialogue_ui_ && dialogue_ui_->has_method("show_message_with_keys")) {
      dialogue_ui_->call("show_message_with_keys", "speaker.system", "System",
                         "correct_evidence", tr_key("correct_evidence"));
    } else if (dialogue_ui_ && dialogue_ui_->has_method("show_message")) {
      dialogue_ui_->call("show_message", "System", tr_key("correct_evidence"));
    }
    testimony_index_++;
    if (testimony_index_ >= testimony_lines_.size()) {
      if (are_all_testimony_contradictions_solved()) {
        complete_testimony(true);
        return;
      }
      testimony_index_ = 0;
    }
    show_current_testimony_line();
    return;
  }

  Node *gs = get_adventure_state();
  if (gs && gs->has_method("take_damage")) {
    gs->call("take_damage");
  }
  if (dialogue_ui_ && dialogue_ui_->has_method("show_message_with_keys")) {
    dialogue_ui_->call("show_message_with_keys", "speaker.system", "System",
                       "wrong_evidence", tr_key("wrong_evidence"));
  } else if (dialogue_ui_ && dialogue_ui_->has_method("show_message")) {
    dialogue_ui_->call("show_message", "System", tr_key("wrong_evidence"));
  }

  int hp = 0;
  if (gs && gs->has_method("get_health")) {
    hp = int(gs->call("get_health"));
  }
  if (hp <= 0) {
    complete_testimony(false);
    return;
  }

  testimony_index_ = 0;
  show_current_testimony_line();
}

void KarakuriScenarioRunner::show_current_testimony_line() {
  if (!testimony_active_ || !testimony_system_ || testimony_lines_.is_empty()) {
    return;
  }
  if (testimony_index_ < 0) {
    testimony_index_ = 0;
  }
  if (testimony_index_ >= testimony_lines_.size()) {
    testimony_index_ = testimony_lines_.size() - 1;
  }

  const Dictionary line = as_dict(testimony_lines_[testimony_index_]);
  const String speaker_key = dict_get_string(line, "speaker_key", "");
  const String speaker = speaker_key.is_empty()
                             ? dict_get_string(line, "speaker_text", "Witness")
                             : tr_key(speaker_key);
  const String text_key = dict_get_string(line, "text_key", "");
  const String text = text_key.is_empty() ? dict_get_string(line, "text_text", "")
                                          : tr_key(text_key);

  if (testimony_system_->has_method("show_panel")) {
    testimony_system_->call("show_panel");
  } else {
    testimony_system_->set("visible", true);
  }
  if (testimony_system_->has_method("show_testimony_line_with_keys")) {
    testimony_system_->call("show_testimony_line_with_keys", speaker_key, speaker,
                            text_key, text);
  } else if (testimony_system_->has_method("show_testimony_line")) {
    testimony_system_->call("show_testimony_line", speaker, text);
  }
  if (testimony_system_->has_method("set_line_progress")) {
    testimony_system_->call("set_line_progress", testimony_index_ + 1,
                            testimony_lines_.size());
  }
}

bool KarakuriScenarioRunner::are_all_testimony_contradictions_solved() const {
  for (int i = 0; i < testimony_lines_.size(); i++) {
    const Dictionary line = as_dict(testimony_lines_[i]);
    const String evidence = dict_get_string(line, "evidence", "");
    const bool solved = dict_get_bool(line, "solved", false);
    if (!evidence.is_empty() && !solved) {
      return false;
    }
  }
  return true;
}

void KarakuriScenarioRunner::complete_testimony(bool success) {
  if (!waiting_for_testimony_) {
    return;
  }
  testimony_active_ = false;
  waiting_for_testimony_ = false;
  waiting_for_evidence_selection_ = false;
  set_mode_input_enabled(false);

  if (evidence_ui_ && evidence_ui_->has_method("hide_inventory")) {
    evidence_ui_->call("hide_inventory");
  }
  if (testimony_system_) {
    if (testimony_system_->has_method("hide_panel")) {
      testimony_system_->call("hide_panel");
    } else {
      testimony_system_->set("visible", false);
    }
  }

  const Array chosen = success ? pending_testimony_success_actions_
                               : pending_testimony_failure_actions_;
  for (int i = chosen.size() - 1; i >= 0; i--) {
    pending_actions_.insert(pending_action_index_, chosen[i]);
  }
}

void KarakuriScenarioRunner::set_mode_input_enabled(bool enabled) {
  mode_input_enabled_ = enabled;
  if (dialogue_ui_ && dialogue_ui_->has_method("set_mode_input_enabled")) {
    dialogue_ui_->call("set_mode_input_enabled", enabled);
  }
  if (evidence_ui_ && evidence_ui_->has_method("set_mode_input_enabled")) {
    evidence_ui_->call("set_mode_input_enabled", enabled);
  }
  if (testimony_system_ &&
      testimony_system_->has_method("set_mode_input_enabled")) {
    testimony_system_->call("set_mode_input_enabled", enabled);
  }
}

void KarakuriScenarioRunner::notify_mode_exit(const String &next_scene_id) {
  if (current_mode_id_.is_empty()) {
    return;
  }
  if (dialogue_ui_ && dialogue_ui_->has_method("on_mode_exit")) {
    dialogue_ui_->call("on_mode_exit", current_mode_id_, next_scene_id);
  }
  if (evidence_ui_ && evidence_ui_->has_method("on_mode_exit")) {
    evidence_ui_->call("on_mode_exit", current_mode_id_, next_scene_id);
  }
  if (testimony_system_ && testimony_system_->has_method("on_mode_exit")) {
    testimony_system_->call("on_mode_exit", current_mode_id_, next_scene_id);
  }
}

void KarakuriScenarioRunner::notify_mode_enter(const String &scene_id,
                                               const Dictionary &scene_dict) {
  current_mode_id_ = resolve_mode_id(scene_id, scene_dict);
  if (dialogue_ui_ && dialogue_ui_->has_method("on_mode_enter")) {
    dialogue_ui_->call("on_mode_enter", current_mode_id_, scene_id);
  }
  if (evidence_ui_ && evidence_ui_->has_method("on_mode_enter")) {
    evidence_ui_->call("on_mode_enter", current_mode_id_, scene_id);
  }
  if (testimony_system_ && testimony_system_->has_method("on_mode_enter")) {
    testimony_system_->call("on_mode_enter", current_mode_id_, scene_id);
  }
  set_mode_input_enabled(true);
}

String KarakuriScenarioRunner::resolve_mode_id(const String &scene_id,
                                               const Dictionary &scene_dict) const {
  const String explicit_mode = dict_get_string(scene_dict, "mode", "");
  if (!explicit_mode.is_empty()) {
    return explicit_mode;
  }
  if (scene_id.find("confrontation") >= 0) {
    return "confrontation";
  }
  if (scene_id.find("deduction") >= 0) {
    return "deduction";
  }
  if (scene_id.find("ending") >= 0) {
    return "ending";
  }
  return "investigation";
}

bool KarakuriScenarioRunner::hotspot_matches_click(const HotspotBinding &hs,
                                                   const Vector2 &pos) const {
  if (!current_scene_instance_) {
    return false;
  }
  Node *n = current_scene_instance_->find_child(hs.node_id, true, false);
  Area2D *area = Object::cast_to<Area2D>(n);
  if (!area) {
    return false;
  }

  const Vector2 hs_pos = area->get_global_position();
  CollisionShape2D *col = find_collision_shape(area);
  if (!col) {
    return false;
  }
  Ref<Shape2D> shape = col->get_shape();
  if (shape.is_null()) {
    return false;
  }

  if (CircleShape2D *c = Object::cast_to<CircleShape2D>(shape.ptr())) {
    const double dist = hs_pos.distance_to(pos);
    return dist <= c->get_radius();
  }

  if (RectangleShape2D *r = Object::cast_to<RectangleShape2D>(shape.ptr())) {
    const Vector2 size = r->get_size();
    const Rect2 rect(hs_pos - size / 2.0, size);
    return rect.has_point(pos);
  }

  return false;
}

void KarakuriScenarioRunner::trigger_hotspot(const HotspotBinding &hs) {
  if (!hs.on_click_actions.is_empty()) {
    start_actions(hs.on_click_actions);
  }
}

String KarakuriScenarioRunner::tr_key(const String &key) const {
  TranslationServer *ts = TranslationServer::get_singleton();
  if (!ts) {
    return key;
  }
  return ts->translate(StringName(key));
}

Node *KarakuriScenarioRunner::resolve_node_path(const NodePath &path) const {
  if (path.is_empty()) {
    return nullptr;
  }
  Node *n = get_node_or_null(path);
  return n;
}

Node *KarakuriScenarioRunner::get_adventure_state() const {
  if (!get_tree()) {
    return nullptr;
  }
  Window *root = get_tree()->get_root();
  if (!root) {
    return nullptr;
  }
  return root->get_node_or_null("AdventureGameState");
}

} // namespace karakuri
