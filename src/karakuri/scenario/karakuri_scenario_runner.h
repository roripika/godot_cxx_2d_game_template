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
 */

#include <godot_cpp/classes/node.hpp>
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
  KarakuriScenarioRunner();
  ~KarakuriScenarioRunner();

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

  bool waiting_for_testimony_ = false;
  godot::Array pending_testimony_success_actions_;
  godot::Array pending_testimony_failure_actions_;

  bool load_scenario();
  bool load_scene_by_id(const godot::String &scene_id);
  void bind_scene_hotspots(const godot::Dictionary &scene_dict);

  void start_actions(const godot::Array &actions);
  void step_actions(double delta);
  bool execute_single_action(const godot::Variant &action);

  void on_clicked_at(const godot::Vector2 &pos);
  void on_choice_selected(int index, const godot::String &text);
  void on_dialogue_finished();
  void on_testimony_complete(bool success);
  bool hotspot_matches_click(const HotspotBinding &hs,
                             const godot::Vector2 &pos) const;
  void trigger_hotspot(const HotspotBinding &hs);

  godot::String tr_key(const godot::String &key) const;
  godot::Node *resolve_node_path(const godot::NodePath &path) const;
  godot::Node *get_adventure_state() const;
};

} // namespace karakuri

#endif // KARAKURI_SCENARIO_RUNNER_H
