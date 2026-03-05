#ifndef KARAKURI_SCENARIO_RUNNER_H
#define KARAKURI_SCENARIO_RUNNER_H

#include <functional>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace karakuri {

/**
 * @brief Testimony state definition (Generic loop control)
 */
struct ScenarioLoopState {
  struct Line {
    godot::String speaker_key;
    godot::String speaker_text;
    godot::String text_key;
    godot::String text_text;
    godot::String evidence_id;
    godot::String shake_key;
    godot::String shake_text;
    bool solved = false;
  };

  godot::Array lines;
  int index = 0;
  int max_rounds = 1;
  int round = 0;
  bool active = false;
  bool waiting = false;
  bool waiting_for_evidence = false;
  godot::Array success_actions;
  godot::Array failure_actions;

  void reset(const godot::String & /*reason*/) {
    active = false;
    waiting = false;
    waiting_for_evidence = false;
    index = 0;
    round = 0;
    max_rounds = 1;
    lines.clear();
    success_actions.clear();
    failure_actions.clear();
  }
};

/**
 * @brief
 * プランナーが記述したYAMLを読み込み、アドベンチャーデモを駆動するシナリオランナー。
 */
class ScenarioRunner : public godot::Node {
  GDCLASS(ScenarioRunner, godot::Node)

public:
  using ActionHandler = std::function<bool(const godot::Variant &payload)>;

  ScenarioRunner();
  ~ScenarioRunner();

  void register_action(const godot::String &kind, ActionHandler handler);
  void register_mystery_actions();

  void set_scenario_path(const godot::String &path);
  godot::String get_scenario_path() const;

  void set_scene_container_path(const godot::NodePath &path);
  godot::NodePath get_scene_container_path() const;

  void set_dialogue_ui_path(const godot::NodePath &path);
  godot::NodePath get_dialogue_ui_path() const;

  void set_evidence_ui_path(const godot::NodePath &path);
  godot::NodePath get_evidence_ui_path() const;

  void set_interaction_manager_path(const godot::NodePath &path);
  godot::NodePath get_interaction_manager_path() const;

  void set_testimony_system_path(const godot::NodePath &path);
  godot::NodePath get_testimony_system_path() const;

  void set_transition_manager_path(const godot::NodePath &path);
  godot::NodePath get_transition_manager_path() const;

  void set_transition_rect_path(const godot::NodePath &path);
  godot::NodePath get_transition_rect_path() const;

  bool is_running() const;
  int get_testimony_index() const;
  int get_testimony_size() const;

  bool get_testimony_active() const;
  godot::String get_current_scene_id() const;

  void _ready() override;
  void _process(double delta) override;

private:
  static void _bind_methods();

  struct HotspotBinding {
    godot::String hotspot_id;
    godot::String node_id;
    godot::Array on_click_actions;
  };

  godot::String scenario_path_ = "res://samples/mystery/scenario/mystery.yaml";
  godot::NodePath scene_container_path_ = godot::NodePath("SceneContainer");
  godot::NodePath dialogue_ui_path_ = godot::NodePath("CanvasLayer/DialogueUI");
  godot::NodePath evidence_ui_path_ =
      godot::NodePath("CanvasLayer/InventoryUI");
  godot::NodePath interaction_manager_path_ =
      godot::NodePath("InteractionManager");
  godot::NodePath testimony_system_path_ =
      godot::NodePath("../CanvasLayer/TestimonySystem");
  godot::NodePath transition_manager_path_ =
      godot::NodePath("TransitionManager");
  godot::NodePath transition_rect_path_ =
      godot::NodePath("TransitionMaskLayer/TransitionRect");

  godot::Dictionary scenario_root_;
  godot::Dictionary scenes_;
  godot::String current_scene_id_;

  godot::Node *scene_container_ = nullptr;
  godot::Node *dialogue_ui_ = nullptr;
  godot::Node *evidence_ui_ = nullptr;
  godot::Node *interaction_manager_ = nullptr;
  godot::Node *testimony_system_ = nullptr;
  godot::Node *transition_manager_ = nullptr;
  godot::Node *transition_rect_ = nullptr;
  godot::Node *current_scene_instance_ = nullptr;

  godot::Array pending_actions_;
  int pending_action_index_ = 0;
  double wait_remaining_sec_ = 0.0;
  bool is_executing_actions_ = false;

  godot::Array hotspot_bindings_;

  bool waiting_for_choice_ = false;
  godot::Array pending_choice_actions_;

  bool waiting_for_dialogue_ = false;
  bool waiting_for_transition_ = false;

  godot::String transition_target_id_;
  float transition_target_duration_ = 0.0f;
  godot::String transition_target_type_;
  float transition_timeout_sec_ = -1.0f;

  ScenarioLoopState testimony_{};

  bool load_scenario();
  bool load_scene_by_id(const godot::String &scene_id);
  void bind_scene_hotspots(const godot::Dictionary &scene_dict);

  void start_actions(const godot::Array &actions);
  void step_actions(double delta);
  bool execute_single_action(const godot::Variant &action);

  void init_builtin_actions();

  void on_clicked_at(const godot::Vector2 &pos);
  void on_choice_selected(int index, const godot::String &text);
  void on_dialogue_finished();
  void on_testimony_complete(bool success);
  void on_testimony_next_requested();
  void on_testimony_shake_requested();
  void on_testimony_present_requested();
  void on_evidence_selected(const godot::String &evidence_id);
  void on_transition_finished(const godot::Variant &arg1 = godot::Variant(),
                              const godot::Variant &arg2 = godot::Variant(),
                              const godot::Variant &arg3 = godot::Variant());
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

  godot::HashMap<godot::String, ActionHandler> action_handlers_;
};

} // namespace karakuri

#endif // KARAKURI_SCENARIO_RUNNER_H
