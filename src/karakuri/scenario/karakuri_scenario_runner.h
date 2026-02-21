#ifndef KARAKURI_SCENARIO_RUNNER_H
#define KARAKURI_SCENARIO_RUNNER_H

/**
 * @file karakuri_scenario_runner.h
 * @brief Basic Game Karakuri:
 * YAML駆動型のシナリオランナー（プランナー作成のストーリーと連携フック）。
 *
 * このノードはシナリオYAMLファイルを読み込み、以下を駆動します:
 * - シーンの構築（コンテナノード配下にインスタンス化）
 * - Hotspotのクリックハンドリング（InteractionManager.clicked_at 経由）
 * - 会話と証拠品の取得（再利用可能なUIノード経由）
 *
 * デザイナーは背景画像やHotspotノードなどのビジュアルを担当します。
 * プランナーは、Hotspot
 * IDやアクション、ストーリーの流れを結びつけるYAMLを担当します。
 *
 * ## アクションのディスパッチ
 * 組み込み汎用アクション（dialogue, goto, choice, set_flag など）は、
 * _init_builtin_actions() で自動的に登録されます。
 *
 * デモ固有のアクション（testimony, take_damage など）は、外部から
 * register_action() を通して
 * 登録することができます。これにより、完全な拡張性を確保しつつ、ランナーのコアを
 * デモ固有のロジックから切り離して維持できます:
 *
 * @code
 * runner->register_action("take_damage", [this](const Variant &payload) {
 *     // デモ固有のハンドラ
 *     return true;
 * });
 * @endcode
 */

#include <functional>

#include "karakuri_testimony_session.h" // Architecture: testimony-session fields will move here

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
 * プランナーが記述したYAMLを読み込み、アドベンチャーデモを駆動するシナリオランナー。
 */
class KarakuriScenarioRunner : public godot::Node {
  GDCLASS(KarakuriScenarioRunner, godot::Node)

public:
  /**
   * @brief 単一のアクションハンドラ用のコールバック（Callable）型。
   *
   * @param payload YAMLのアクションディクショナリの値部分
   *                （型はアクションに依存します: String, Dictionary,
   * Arrayなど）。
   * @return
   * アクションが処理された場合はtrue。不明なアクション警告を出す場合はfalse。
   */
  using ActionHandler = std::function<bool(const godot::Variant &payload)>;

  KarakuriScenarioRunner();
  ~KarakuriScenarioRunner();

  /**
   * @brief YAMLアクション種別に対するハンドラを登録（または上書き）する。
   *
   * 親や兄弟の _ready() からこれを呼び出し、ランナーのコアを変更せずに
   * デモ固有のアクションを注入します。必要であれば、組み込みの汎用アクションも
   * この方法で上書き可能です。
   *
   * @param kind    YAMLに記述されるアクションのキー（例: "take_damage"）。
   * @param handler アクションのペイロードを処理するコールバック関数。
   */
  void register_action(const godot::String &kind, ActionHandler handler);

  /**
   * @brief ミステリーデモ用のアクションセット（testimony, take_damage,
   *        if_health_ge, if_health_leq）を登録する。
   *
   * ミステリーデモのシェルの _ready() から一度だけ TestimonySystem 初期化後に
   * 呼び出してください。他のデモからは呼び出さないでください。
   */
  void register_mystery_actions();

  /** @brief YAMLシナリオファイルのパス (res://...)。 */
  void set_scenario_path(const godot::String &path);
  godot::String get_scenario_path() const;

  /** @brief 基本シーンがインスタンス化されるシーンコンテナノードのNodePath。 */
  void set_scene_container_path(const godot::NodePath &path);
  godot::NodePath get_scene_container_path() const;

  /** @brief show_message(speaker, text) をサポートするDialogue
   * UIノードのNodePath。 */
  void set_dialogue_ui_path(const godot::NodePath &path);
  godot::NodePath get_dialogue_ui_path() const;

  /** @brief add_evidence(evidence_id)
   * をサポートする証拠品インベントリUIノードのNodePath。 */
  void set_evidence_ui_path(const godot::NodePath &path);
  godot::NodePath get_evidence_ui_path() const;

  /**
   * @brief clicked_at(Vector2)
   * シグナルを発火するInteractionManagerノードのNodePath。
   *
   * 期待される座標空間は Canvas/World 座標です
   * (読み込まれたMysteryシーン内の `Area2D.global_position`
   * と互換性があります)。
   */
  void set_interaction_manager_path(const godot::NodePath &path);
  godot::NodePath get_interaction_manager_path() const;

  /** @brief TestimonySystem ノードへのNodePath
   * (任意、コンフロンテーションモード用)。 */
  void set_testimony_system_path(const godot::NodePath &path);
  godot::NodePath get_testimony_system_path() const;

  /** @brief Godotのライフサイクルフック。 */
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
  godot::Dictionary scenes_; // scene_id -> scene dict
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
  godot::Array pending_choice_actions_; // Array<Array<action>>

  /**
   * @brief Dialogue UI
   * がメッセージの表示を完了するのを待機しているかを示すフラグ。
   *
   * Dialogue UI が `dialogue_finished` シグナルを提供している場合、ランナーは
   * 複数の `dialogue` アクションがキューに入れられているときにテキストが即座に
   * 上書きされるのを防ぐため、シグナルが発火するまで後続のアクションをブロックします。
   */
  bool waiting_for_dialogue_ = false;

  /**
   * @brief Transition Manager
   * がエフェクト（Tween）の完了を待機しているかを示すフラグ。
   */
  bool waiting_for_transition_ = false;

  /**
   * @brief Mysteryの証言コンフロンテーションセッションのステート。
   *
   * 証言ループの全フィールドをカプセル化しています。karakuri_testimony_session.h
   * を参照。 testimony_.active, testimony_.index などでアクセスします。
   */
  KarakuriTestimonySession testimony_{}; // @see KarakuriTestimonySession

  bool load_scenario();
  bool load_scene_by_id(const godot::String &scene_id);
  void bind_scene_hotspots(const godot::Dictionary &scene_dict);

  void start_actions(const godot::Array &actions);
  void step_actions(double delta);
  bool execute_single_action(const godot::Variant &action);

  /** @brief 全ての組み込み汎用アクションを action_handlers_ に登録する。 */
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
                              const godot::Variant &arg2 = godot::Variant());
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

  /** @brief ディスパッチテーブル: YAMLのアクション種別キー -> ハンドラ関数。 */
  godot::HashMap<godot::String, ActionHandler> action_handlers_;
};

} // namespace karakuri

#endif // KARAKURI_SCENARIO_RUNNER_H
