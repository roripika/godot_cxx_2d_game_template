#ifndef KARAKURI_GAME_STATE_H
#define KARAKURI_GAME_STATE_H

/**
 * @file karakuri_game_state.h
 * @brief Basic Game Karakuri: ゲーム基盤ステート管理。
 *
 * ## 責務（Karakuri 境界内）
 * - ゲームリセットフックの DI（Mystery 等の上位層が登録する Callable）
 * - reset_game() の呼び出しとフック実行
 *
 * ## 責務外（境界外 → 継承クラスへ）
 * - HP 管理 → mystery::MysteryGameState
 * - シーン遷移 → karakuri::SceneFlow
 * - フラグ管理 → karakuri::FlagService
 *
 * ## 使い方
 * - Godot Autoload として /root/KarakuriGameState に配置する。
 * - Mystery テンプレートを使う場合は代わりに MysteryGameState を Autoload にする。
 *   MysteryGameState のコンストラクタが KarakuriGameState::get_singleton() も更新するため
 *   どちらの名前でも参照できる。
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace karakuri {

class KarakuriGameState : public godot::Node {
  GDCLASS(KarakuriGameState, godot::Node)

protected:
  /// サブクラスから参照できるよう protected にする
  static KarakuriGameState *singleton_;

  /// Mystery 等の上層が登録するリセットフック（Callable / invalid == 未登録）
  godot::Callable reset_hook_;

  /// 汎用フラグストア。条件評価エンジン (ConditionEvaluator) が参照する。
  godot::Dictionary global_flags_;

  static void _bind_methods();

public:
  KarakuriGameState();
  ~KarakuriGameState() override;

  /** @brief 唯一のインスタンスを返す（Autoload 前提）。 */
  static KarakuriGameState *get_singleton();

  // ------------------------------------------------------------------
  // リセット API
  // ------------------------------------------------------------------

  /**
   * @brief ゲームをリセットする。
   * reset_hook_ が登録されていれば先に呼び出す。
   * サブクラスはこれをオーバーライドして追加初期化を行うこと。
   */
  virtual void reset_game();

  /**
   * @brief Mystery 等の上位層がリセット時の追加処理を注入するフックを登録する。
   * _ready() 等で一度だけ呼ぶ。
   */
  void set_reset_hook(const godot::Callable &hook);

  // ------------------------------------------------------------------
  // 汎用フラグ API
  // ------------------------------------------------------------------

  /** @brief フラグに任意の値をセットする。 */
  void set_flag(const godot::String &key, const godot::Variant &value);

  /** @brief フラグの値を返す。未設定の場合は default_value を返す。 */
  godot::Variant get_flag(const godot::String &key,
                          const godot::Variant &default_value = godot::Variant()) const;

  /** @brief フラグが存在するか判定する。 */
  bool has_flag(const godot::String &key) const;
};

} // namespace karakuri

#endif // KARAKURI_GAME_STATE_H
