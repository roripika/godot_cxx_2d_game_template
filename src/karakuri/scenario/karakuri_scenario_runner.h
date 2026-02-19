#ifndef KARAKURI_SCENARIO_RUNNER_H
#define KARAKURI_SCENARIO_RUNNER_H

/**
 * @file karakuri_scenario_runner.h
 * @brief Basic Game Karakuri: YAML-driven scenario runner (planner-authored story + hooks).
 *
 * This node loads a scenario YAML file and drives:
 * - Scene composition (instantiate under a container node)
 * - Hotspot click handling (via InteractionManager.clicked_at)
 * - Dialogue and evidence acquisition (via reusable UI nodes)
 *
 * Designers own the visuals and hotspot nodes in the base scenes.
 * Planners own the YAML that binds hotspot IDs to actions and story flow.
 *
 * ## Action dispatch
 * Built-in generic actions (dialogue, goto, choice, set_flag, …) are
 * registered automatically in _init_builtin_actions().
 *
 * Demo-specific actions (testimony, take_damage, …) can be registered from
 * outside via register_action().  This keeps the runner core free of
 * demo-specific logic while still allowing full extensibility:
 *
 * @code
 * runner->register_action("take_damage", [this](const Variant &payload) {
 *     // demo-specific handler
 *     return true;
 * });
 * @endcode
 */

#include <functional>

#include "karakuri_testimony_session.h"  // Architecture: testimony-session fields will move here

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace karakuri {

/**
 * @brief Scenario runner that reads planner YAML and drives the adventure demo.
 */
class KarakuriScenarioRunner : public godot::Node {
  GDCLASS(KarakuriScenarioRunner, godot::Node)

public:
  /**
   * @brief Callable type for a single action handler.
   *
   * @param payload The value portion of the YAML action dictionary
   *                (type varies per action: String, Dictionary, Array, ...).
   * @return true if the action was handled, false to emit an unknown-action warning.
   */
  using ActionHandler = std::function<bool(const godot::Variant &payload)>;

  KarakuriScenarioRunner();
  ~KarakuriScenarioRunner();

  /**
   * @brief Register (or override) a handler for a YAML action kind.
   *
   * Call this from a parent/sibling _ready() to inject demo-specific actions
   * without modifying the runner core.  Built-in generic actions can also be
   * overridden this way if needed.
   *
   * @param kind    The action key as it appears in YAML (e.g. "take_damage").
   * @param handler Callable that processes the action payload.
   */
  void register_action(const godot::String &kind, ActionHandler handler);

  /**
   * @brief Register the Mystery-demo action set (testimony, take_damage,
   *        if_health_ge, if_health_leq).
   *
   * Call once from the Mystery shell scene's _ready() after initialising
   * TestimonySystem.  Other demos should NOT call this.
   */
  void register_mystery_actions();

  /** @brief Path to the YAML scenario file (res://...). */
  void set_scenario_path(const godot::String &path);
  godot::String get_scenario_path() const;

  /** @brief NodePath for the scene container where base scenes are instantiated. */
  void set_scene_container_path(const godot::NodePath &path);
  godot::NodePath get_scene_container_path() const;

  /** @brief NodePath to the Dialogue UI node that supports show_message(speaker, text). */
  void set_dialogue_ui_path(const godot::NodePath &path);
  godot::NodePath get_dialogue_ui_path() const;

  /** @brief NodePath to the Evidence inventory UI node that supports add_evidence(evidence_id). */
  void set_evidence_ui_path(const godot::NodePath &path);
  godot::NodePath get_evidence_ui_path() const;

  /**
   * @brief NodePath to InteractionManager node that emits clicked_at(Vector2).
   *
   * Expected coordinate space is Canvas/World coordinates (compatible with
   * `Area2D.global_position` in loaded mystery scenes).
   */
  void set_interaction_manager_path(const godot::NodePath &path);
  godot::NodePath get_interaction_manager_path() const;

  /** @brief NodePath to TestimonySystem node (optional, for confrontation mode). */
  void set_testimony_system_path(const godot::NodePath &path);
  godot::NodePath get_testimony_system_path() const;

  /** @brief Godot lifecycle hook. */
  void _ready() override;
  /** @brief Godot lifecycle hook. */
  void _process(double delta) override;

private:
  static void _bind_methods();

  struct HotspotBinding {
    godot::String hotspot_id;
    godot::String node_id;
    godot::Array on_click_actions;
  };

  godot::String scenario_path_ =
      "res://samples/mystery/scenario/mystery.yaml";
  godot::NodePath scene_container_path_ = godot::NodePath("SceneContainer");
  godot::NodePath dialogue_ui_path_ =
      godot::NodePath("CanvasLayer/DialogueUI");
  godot::NodePath evidence_ui_path_ =
      godot::NodePath("CanvasLayer/InventoryUI");
  godot::NodePath interaction_manager_path_ =
      godot::NodePath("InteractionManager");
  godot::NodePath testimony_system_path_ =
      godot::NodePath("../CanvasLayer/TestimonySystem");

  godot::Dictionary scenario_root_;
  godot::Dictionary scenes_; // scene_id -> scene dict
  godot::String current_scene_id_;

  godot::Node *scene_container_ = nullptr;
  godot::Node *dialogue_ui_ = nullptr;
  godot::Node *evidence_ui_ = nullptr;
  godot::Node *interaction_manager_ = nullptr;
  godot::Node *testimony_system_ = nullptr;
  godot::Node *current_scene_instance_ = nullptr;

  godot::Array pending_actions_;
  int pending_action_index_ = 0;
  double wait_remaining_sec_ = 0.0;
  bool is_executing_actions_ = false;

  godot::Array hotspot_bindings_;

  bool waiting_for_choice_ = false;
  godot::Array pending_choice_actions_; // Array<Array<action>>

  /**
   * @brief True while waiting for Dialogue UI to finish presenting a message.
   *
   * If the Dialogue UI provides a `dialogue_finished` signal, the runner will
   * block subsequent actions until it fires, to avoid fast "overwrite" of
   * dialogue text when multiple `dialogue` actions are queued.
   */
  bool waiting_for_dialogue_ = false;

  /**
   * @brief Mystery testimony confrontation session state.
   *
   * Encapsulates all testimony-loop fields. See karakuri_testimony_session.h.
   * Access via testimony_.active, testimony_.index, etc.
   */
  KarakuriTestimonySession testimony_{};  // @see KarakuriTestimonySession

  bool load_scenario();
  bool load_scene_by_id(const godot::String &scene_id);
  void bind_scene_hotspots(const godot::Dictionary &scene_dict);

  void start_actions(const godot::Array &actions);
  void step_actions(double delta);
  bool execute_single_action(const godot::Variant &action);

  /** @brief Register all built-in generic actions into action_handlers_. */
  void init_builtin_actions();

  void on_clicked_at(const godot::Vector2 &pos);
  void on_choice_selected(int index, const godot::String &text);
  void on_dialogue_finished();
  void on_testimony_complete(bool success);
  void on_testimony_next_requested();
  void on_testimony_shake_requested();
  void on_testimony_present_requested();
  void on_evidence_selected(const godot::String &evidence_id);
  bool hotspot_matches_click(const HotspotBinding &hs,
                             const godot::Vector2 &pos) const;
  void trigger_hotspot(const HotspotBinding &hs);
  void show_current_testimony_line();
  bool are_all_testimony_contradictions_solved() const;
  void complete_testimony(bool success);

  void set_mode_input_enabled(bool enabled);
  void notify_mode_exit(const godot::String &next_scene_id);
  void notify_mode_enter(const godot::String &scene_id,
                         const godot::Dictionary &scene_dict);
  godot::String resolve_mode_id(const godot::String &scene_id,
                                const godot::Dictionary &scene_dict) const;

  godot::String tr_key(const godot::String &key) const;
  godot::Node *resolve_node_path(const godot::NodePath &path) const;
  godot::Node *get_adventure_state() const;

  godot::String current_mode_id_;
  bool mode_input_enabled_ = true;

  /** @brief Dispatch table: YAML action kind -> handler function. */
  godot::HashMap<godot::String, ActionHandler> action_handlers_;
};

} // namespace karakuri

#endif // KARAKURI_SCENARIO_RUNNER_H
