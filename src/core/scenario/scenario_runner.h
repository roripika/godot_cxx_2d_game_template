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
 * @brief
 * 汎用シナリオエンジン。アクションディスパッチャーパターンにより、
 * ジャンル固有のロジック（ミステリーの証言等）は外部から登録される。
 */
class ScenarioRunner : public godot::Node {
  GDCLASS(ScenarioRunner, godot::Node)

public:
  using ActionHandler = std::function<bool(const godot::Variant &payload)>;

  ScenarioRunner();
  ~ScenarioRunner();

  /**
   * @brief 外部からカスタムアクションを登録する。
   */
  void register_action(const godot::String &kind, ActionHandler handler);

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

  void set_transition_manager_path(const godot::NodePath &path);
  godot::NodePath get_transition_manager_path() const;

  void set_transition_rect_path(const godot::NodePath &path);
  godot::NodePath get_transition_rect_path() const;

  bool is_running() const;
  godot::String get_current_scene_id() const;

  void _ready() override;
  void _process(double delta) override;

  // 公開メソッド (GDScriptからの呼び出し用)
  void load_scenario();
  void load_scene_by_id(const godot::String &scene_id);
  bool execute_single_action(const godot::Variant &action);
  void complete_custom_action();

  /**
   * @brief 現在のアクション直後にステップを挿入する。
   *
   * present_evidence 等の分岐アクションから呼び出すことで、
   * on_correct / on_wrong などのサブ配列をキューに挿入できる。
   *
   * @param steps 挿入するアクション配列。
   */
  void inject_steps(const godot::Array &steps);

protected:
  static void _bind_methods();

private:
  struct HotspotBinding {
    godot::String hotspot_id;
    godot::String node_id;
    godot::Array on_click_actions;
  };

  godot::String scenario_path_ = "";
  godot::NodePath scene_container_path_ = godot::NodePath("SceneContainer");
  godot::NodePath dialogue_ui_path_ = godot::NodePath("CanvasLayer/DialogueUI");
  godot::NodePath evidence_ui_path_ =
      godot::NodePath("CanvasLayer/InventoryUI");
  godot::NodePath interaction_manager_path_ =
      godot::NodePath("InteractionManager");
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
  bool waiting_for_custom_action_ = false;

  godot::String transition_target_id_;
  float transition_target_duration_ = 0.0f;
  godot::String transition_target_type_;
  float transition_timeout_sec_ = -1.0f;

  bool load_scenario_internal();
  void bind_scene_hotspots(const godot::Dictionary &scene_dict);

  void start_actions(const godot::Array &actions);
  void step_actions(double delta);

  void init_builtin_actions();

  // シグナルハンドラ
  void on_clicked_at(const godot::Vector2 &pos);
  void on_choice_selected(int index, const godot::String &text);
  void on_dialogue_finished();
  void on_evidence_selected(const godot::String &evidence_id);
  void on_transition_finished(const godot::Variant &arg1 = godot::Variant(),
                              const godot::Variant &arg2 = godot::Variant(),
                              const godot::Variant &arg3 = godot::Variant());

  bool hotspot_matches_click(const HotspotBinding &hs,
                             const godot::Vector2 &pos) const;
  void trigger_hotspot(const HotspotBinding &hs);

  void set_mode_input_enabled(bool enabled);
  void notify_mode_exit(const godot::String &current_mode,
                        const godot::String &next_scene_id);
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
