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
 * - `scene_replaced(from_path, to_path)`
 * - `scene_pushed(from_path, to_path)`
 * - `scene_popped(from_path, to_path)` : from=元いた画面, to=戻り先
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class SceneFlow : public godot::Node {
  GDCLASS(SceneFlow, godot::Node)

  static SceneFlow *singleton_;

  /// スタックの各エントリ: { "path": String, "params": Dictionary }
  godot::Array history_stack_;

  /// 現在のシーンパス（遷移後に更新）
  godot::String current_path_;

  /// 遷移先がシーンから受け取れるパラメータ
  godot::Dictionary current_params_;

  /// 実際の SceneTree::change_scene_to_file を呼ぶ内部メソッド
  void do_change_scene(const godot::String &path);

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
   * @brief 現在シーンを履歴スタックに積んで新しいシーンへ移る。
   * ポーズメニューやサブ画面のような「戻れる遷移」に使う。
   */
  void push_scene(const godot::String &path,
                  const godot::Dictionary &params = godot::Dictionary());

  /**
   * @brief スタックから直前のシーンに戻る。
   * スタックが空の場合は何もしない。
   */
  void pop_scene();

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

  /** @brief 履歴スタックを全クリアする。ゲームオーバー等でリセット用。 */
  void clear_history();
};

} // namespace karakuri

#endif // KARAKURI_SCENE_FLOW_H
