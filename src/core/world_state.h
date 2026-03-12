#ifndef KARAKURI_WORLD_STATE_H
#define KARAKURI_WORLD_STATE_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace karakuri {

/**
 * @brief
 * Karakuri OS のメモリサンドボックス機構 (旧 FlagService)。
 * 単一のグローバル辞書による状態汚染を防ぐため、名前空間（Namespace）と
 * 生存期間（Scope）による厳格なメモリ管理・分離を行う。
 *
 * ## 内部データ構造
 * スコープごとに独立した Dictionary を持ち、その中で Namespace により
 * ツリー化する: { namespace: { key: value } }
 *
 * ## 後方互換 (旧 FlagService)
 * - set_flag / get_flag / has_flag を namespace="core", scope=SCOPE_GLOBAL で
 *   ラップするメソッドとして提供。
 */
class WorldState : public godot::Node {
  GDCLASS(WorldState, godot::Node)

public:
  /**
   * @brief メモリスコープの定義。
   * SCOPE_GLOBAL : アプリケーション全体で永続（セーブ対象）
   * SCOPE_SESSION: 1回のプレイセッション中のみ有効（タイトルへのリセット時などで破棄）
   * SCOPE_SCENE  : 現在のシーン内のみ有効（シーン遷移時に自動破棄）
   */
  enum Scope {
    SCOPE_GLOBAL  = 0,
    SCOPE_SESSION = 1,
    SCOPE_SCENE   = 2
  };

  WorldState();
  ~WorldState() override;

  static WorldState *get_singleton();

  /**
   * @brief 指定した名前空間・スコープにステート（値）を設定する。
   * シグナル "state_changed(namespace, scope, key, value)" が発火する。
   */
  void set_state(const godot::String &ns, int scope,
                 const godot::String &key, const godot::Variant &value);

  /**
   * @brief 指定した名前空間・スコープから値を取得する。
   */
  godot::Variant get_state(const godot::String &ns, int scope,
                           const godot::String &key,
                           const godot::Variant &default_val = godot::Variant()) const;

  /**
   * @brief ブール値フラグとして評価するヘルパー関数。
   */
  bool has_flag(const godot::String &ns, int scope,
                const godot::String &key) const;

  /**
   * @brief 指定したスコープのメモリを解放（ガベージコレクション）する。
   * 例: シーン遷移時に clear_scope(SCOPE_SCENE) を呼び出す。
   */
  void clear_scope(int scope);

  /**
   * @brief 特定の名前空間の全スコープのメモリを解放する（モジュールのアンロード時など）。
   */
  void clear_namespace(const godot::String &ns);

  /**
   * @brief 永続化すべき SCOPE_GLOBAL の状態をシリアライズ/デシリアライズする。
   * セーブ・ロードの基盤として使用。
   */
  godot::Dictionary serialize_globals() const;
  void deserialize_globals(const godot::Dictionary &data);

  // ------------------------------------------------------------------
  // 後方互換ラッパー (旧 FlagService API 相当)
  // namespace="core", scope=SCOPE_GLOBAL を固定で使用。
  // ------------------------------------------------------------------
  void set_flag(const godot::String &key, const godot::Variant &value);
  godot::Variant get_flag(const godot::String &key,
                          const godot::Variant &default_val = godot::Variant()) const;
  bool has_flag_simple(const godot::String &key) const;
  void erase_flag(const godot::String &key);

protected:
  static void _bind_methods();

private:
  static WorldState *singleton_;

  // 内部データ構造: Scope ごとに独立した辞書を持ち、その中で Namespace によりツリー化する
  // { namespace: { key: value } }
  godot::Dictionary global_states_;
  godot::Dictionary session_states_;
  godot::Dictionary scene_states_;

  godot::Dictionary       *get_scope_dict(int scope);
  const godot::Dictionary *get_scope_dict(int scope) const;
};

} // namespace karakuri

// GDScript 用に Enum をバインド
VARIANT_ENUM_CAST(karakuri::WorldState::Scope);

#endif // KARAKURI_WORLD_STATE_H
