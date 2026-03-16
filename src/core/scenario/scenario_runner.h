#ifndef KARAKURI_SCENARIO_RUNNER_H
#define KARAKURI_SCENARIO_RUNNER_H

#include <functional>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include "../tasks/task_base.h"

namespace karakuri {

struct CompiledScene {
  godot::Array tasks;
  godot::Dictionary raw_dict;
};

/**
 * @brief
 * 汎用シナリオエンジン。アクションディスパッチャーパターンにより、
 * ジャンル固有のロジック（ミステリーの証言等）は外部から登録される。
 */
class ScenarioRunner : public godot::Node {
  GDCLASS(ScenarioRunner, godot::Node)

public:

  ScenarioRunner();
  ~ScenarioRunner();


  void set_scenario_path(const godot::String &path);
  godot::String get_scenario_path() const;

  bool is_running() const;
  godot::String get_current_scene_id() const;

  /**
   * @brief 現在実行中の pending_action_index_ を返す。
   * SequencePlayer::create_snapshot() からスナップショット保存に使用する。
   */
  int get_pending_action_index() const;

  /**
   * @brief ScenarioRunner の状態を指定シーン・コマンドインデックスに復元する。
   * SequencePlayer::rollback_to_last_snapshot() から呼ばれる。
   * シーン遷移が必要な場合は load_scene_by_id() を内部で呼ぶ。
   *
   * @param scene_id     復元先のシーン ID
   * @param action_index 復元先の pending_action_index_
   */
  void restore_to(const godot::String &scene_id, int action_index);

  void _ready() override;
  void _process(double delta) override;

  // 公開メソッド (GDScriptからの呼び出し用)
  void load_scenario();
  void load_scene_by_id(const godot::String &scene_id);
  void complete_custom_action();
  void complete_transition();
  void request_transition(const godot::String &target_scene, const godot::Dictionary &params);

  /**
   * @brief シナリオ（台詞）を一行進める。
   * View (GDScript) からのクリック等で呼ばれる。
   */
  void advance_dialogue();

  /**
   * @brief 選択肢（Choice）を確定する。
   * @param index 選択されたインデックス
   */
  void submit_choice(int index);

  /**
   * @brief 現在のアクション直後にステップを挿入する。
   *
   * present_evidence 等の分岐アクションから呼び出すことで、
   * on_correct / on_wrong などのサブ配列をキューに挿入できる。
   *
   * @param steps 挿入するアクション配列。
   */
  void inject_steps(const godot::Array &steps);

  // Task 向けのアクセス API
  bool is_waiting_for_dialogue() const { return waiting_for_dialogue_; }
  void set_waiting_for_dialogue(bool w) { waiting_for_dialogue_ = w; }
  
  bool is_waiting_for_choice() const { return waiting_for_choice_; }
  void set_waiting_for_choice(bool w) { waiting_for_choice_ = w; }
  void set_pending_choice_actions(const godot::Array &a) { pending_choice_actions_ = a; }

  bool is_waiting_for_transition() const { return waiting_for_transition_; }

protected:
  static void _bind_methods();

private:
  godot::String scenario_path_ = "";

  godot::Dictionary scenario_root_;
  godot::HashMap<godot::String, CompiledScene> scenes_;
  godot::String current_scene_id_;

  godot::Array pending_actions_; // Ref<TaskBase> の配列
  int pending_action_index_ = 0;
  double wait_remaining_sec_ = 0.0;
  bool is_executing_actions_ = false;

  bool waiting_for_choice_ = false;
  godot::Array pending_choice_actions_;

  bool waiting_for_dialogue_ = false;
  bool waiting_for_transition_ = false;
  double transition_start_time_ = 0.0;
  bool waiting_for_custom_action_ = false;

  bool load_scenario_internal();
  godot::Ref<TaskBase> compile_action(const godot::Variant &v);

  void start_actions(const godot::Array &actions);
  void step_actions();

  void set_mode_input_enabled(bool enabled);
  void notify_mode_exit(const godot::String &current_mode,
                        const godot::String &next_scene_id);
  void notify_mode_enter(const godot::String &scene_id,
                         const godot::Dictionary &scene_dict);
  godot::String resolve_mode_id(const godot::String &scene_id,
                                const godot::Dictionary &scene_dict) const;

  godot::String current_mode_id_;
  bool mode_input_enabled_ = true;

};

} // namespace karakuri

#endif // KARAKURI_SCENARIO_RUNNER_H
