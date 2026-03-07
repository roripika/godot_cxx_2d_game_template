#ifndef KARAKURI_SCENE_FLOW_H
#define KARAKURI_SCENE_FLOW_H

/**
 * @file scene_flow.h
 * @brief Basic Game Karakuri: 汎用シーン遷移スタック管理サービス。
 *
 * ## 役割
 * `push`, `pop`, `replace` の3つのAPIで画面スタックを管理する。
 * Autoload として /root に配置することで、全シーンから参照できるシングルトンになる。
 *
 * ## push / pop / replace の使い分け
 *
 * ### replace_scene(path, params)
 * 履歴に残さず現在のシーンを置き換える。タイトル → ゲーム本編のように
 * 「戻る必要がない遷移」に使う。
 *
 * ### push_scene(path, params)
 * 現在のシーンパスを履歴スタックに積んでから新しいシーンに移る。
 * ゲーム本編 → ポーズメニューのように「戻れる遷移」に使う。
 *
 * ### pop_scene()
 * スタックから直前のシーンパスを取り出し、そのシーンに戻る。
 * スタックが空のときは何もしない（`can_pop()` で事前確認推奨）。
 *
 * ## パラメータ渡し（params）
 * 遷移先のシーンは `SceneFlow::get_singleton()->get_current_params()` を
 * 呼ぶことで、呼び出し元が渡した Dictionary を受け取れる。
 *
 * ## 使い方（GDScript）
 * ```gdscript
 * # 戻れない遷移
 * SceneFlow.replace_scene("res://scenes/game.tscn")
 *
 * # 戻れる遷移（ポーズなど）
 * SceneFlow.push_scene("res://scenes/pause_menu.tscn", {"from": "game"})
 * # ...ポーズ解除
 * SceneFlow.pop_scene()
 *
 * # 遷移先でパラメータを受け取る
 * var params = SceneFlow.get_current_params()
 * ```
 *
 * ## シグナル
 * ### 遷移イベント（スタック変化）
 * - `scene_replaced(from_path, to_path)`
 * - `scene_pushed(from_path, to_path)`
 * - `scene_popped(from_path, to_path)` : from=元いた画面, to=戻り先
 *
 * ### 演出フック（フェードイン/アウト連携用）
 * - `transition_started(to_path)` : 遷移直前に発火。GDScript 側でフェードアウトを開始する。
 * - `transition_finished()`        : do_change_scene() 直後に発火。フェードイン開始の合図。
 *
 * ## `return_to_title()` について
 * `title_scene_path` プロパティに登録したシーンへ、履歴を全クリアして戻る。
 * タイトルへの「強制脱出」として使う。
 */

#include <vector>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class SceneFlow : public godot::Node {
  GDCLASS(SceneFlow, godot::Node)

  // ------------------------------------------------------------------
  // ノード保持型スタックエントリ
  // ------------------------------------------------------------------
  /// push_scene で suspend したシーンのレコード
  struct SceneRecord {
    godot::String     path;             ///< シーンリソースパス
    godot::Dictionary params;           ///< 渡されたパラメータ
    godot::Node*      suspended_scene;  ///< remove_child() で取り外したノード（解放しない！）
  };

  static SceneFlow *singleton_;

  /// push されて一時停止中のシーンのスタック（後入れ先出し）
  std::vector<SceneRecord> history_stack_;

  /// push_scene で表示中のシーンノード。
  /// replace_scene 直後（push 前）は nullptr（SceneTree が管理）。
  godot::Node* active_pushed_scene_ = nullptr;

  /// 現在のシーンパス（遷移後に更新）
  godot::String current_path_;

  /// 遷移先がシーンから受け取れるパラメータ
  godot::Dictionary current_params_;

  /// return_to_title() で戻る先のパス（インスペクタまたはコードから設定）
  godot::String title_scene_path_;

  /// replace_scene / return_to_title 専用: change_scene_to_file + シグナル
  void do_replace_scene(const godot::String &path);

protected:
  static void _bind_methods();

public:
  SceneFlow();
  ~SceneFlow() override;

  static SceneFlow *get_singleton();

  // ------------------------------------------------------------------
  // 遷移 API
  // ------------------------------------------------------------------

  /**
   * @brief 履歴に残さず現在シーンを置き換える。
   * タイトル→ゲームのような「戻らない遷移」に使う。
   */
  void replace_scene(const godot::String &path,
                     const godot::Dictionary &params = godot::Dictionary());

  /**
   * @brief 現在シーンをメモリ上に保持したまま新しいシーンへ移る。
   *
   * 現在のノードを SceneTree から remove_child()（= 凍結）し、
   * 新しいシーンを即座に instantiate() して add_child() する。
   * pop_scene() で凍結したノードをそのまま復元できる（リロード不要）。
   */
  void push_scene(const godot::String &path,
                  const godot::Dictionary &params = godot::Dictionary());

  /**
   * @brief 現在のシーンを破棄し、スタックに保持していた直前のシーンを復元する。
   *
   * 現在シーンを queue_free() し、suspend 中のノードを add_child() で再接続する。
   * スタックが空の場合は何もしない。
   */
  void pop_scene();

  /**
   * @brief 履歴スタックを全クリアし、title_scene_path へ遷移する。
   * ゲームオーバーやタイトルへの「強制脱出」に使う。
   * title_scene_path が空の場合は警告を出して何もしない。
   */
  void return_to_title();

  // ------------------------------------------------------------------
  // 参照 API
  // ------------------------------------------------------------------

  /** @brief 戻れる履歴が残っているか。 */
  bool can_pop() const;

  /** @brief 現在のシーンパスを返す。 */
  godot::String get_current_path() const;

  /**
   * @brief 遷移時に渡されたパラメータを返す。
   * 遷移先の _ready() から呼ぶことで呼び出し元のデータを受け取れる。
   */
  godot::Dictionary get_current_params() const;

  /** @brief 履歴スタックの深さ（push した回数）を返す。 */
  int get_stack_depth() const;

  /** @brief スタックに保持している全シーンノードを queue_free し、履歴をクリアする。 */
  void clear_history();

  // ------------------------------------------------------------------
  // title_scene_path プロパティ
  // ------------------------------------------------------------------
  void set_title_scene_path(const godot::String &path);
  godot::String get_title_scene_path() const;
};

} // namespace karakuri

#endif // KARAKURI_SCENE_FLOW_H
