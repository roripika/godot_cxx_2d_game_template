#ifndef KARAKURI_FLAG_SERVICE_H
#define KARAKURI_FLAG_SERVICE_H

/**
 * @file flag_service.h
 * @brief Basic Game Karakuri: 汎用フラグ管理サービス。
 *
 * ## 役割
 * - ゲーム内の全フラグ（Boolean / Integer / String / Float）を
 *   型に縛られない Variant として一元管理するシングルトン。
 * - SaveService と連携し、serialize()/deserialize() でフラグを永続化。
 * - ConditionEvaluator が参照する唯一のフラグストアとして機能する。
 *
 * ## 設計ポリシー (karakuri 層)
 * - ミステリー固有の概念（証拠品・証言等）を一切知らない。
 * - MysteryManager の既存フラグ（flags_）と互換させるため
 *   MysteryManager から deserialize 時に sync_from() で移行可能。
 *
 * ## 使い方
 * ```cpp
 * auto *fs = karakuri::FlagService::get_singleton();
 * fs->set_flag("is_cabinet_opened", true);
 * bool open = bool(fs->get_flag("is_cabinet_opened")); // true
 * fs->set_flag("player_level", 5);
 * ```
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace karakuri {

class FlagService : public godot::Node {
  GDCLASS(FlagService, godot::Node)

private:
  static FlagService *singleton_;
  /// @brief フラグストア: { name: Variant }
  godot::Dictionary flags_;

  static void _bind_methods();

public:
  FlagService();
  ~FlagService() override;

  static FlagService *get_singleton();

  // ------------------------------------------------------------------
  // フラグ操作
  // ------------------------------------------------------------------

  /**
   * @brief フラグを設定する。値は任意の Variant 型が使える。
   */
  void set_flag(const godot::String &name, const godot::Variant &value);

  /**
   * @brief フラグ値を取得する。未設定なら p_default を返す。
   */
  godot::Variant get_flag(const godot::String &name,
                          const godot::Variant &p_default = godot::Variant()) const;

  /**
   * @brief フラグが設定済みか確認する。
   */
  bool has_flag(const godot::String &name) const;

  /**
   * @brief フラグを削除する。
   */
  void erase_flag(const godot::String &name);

  /**
   * @brief 全フラグをクリアする。
   */
  void clear_flags();

  /**
   * @brief 設定されているフラグ名の一覧を返す。
   */
  godot::Array get_flag_names() const;

  /**
   * @brief 既存の Dictionary をまとめてマージする（上書き）。
   * MysteryManager::flags_ からの移行等に使用。
   */
  void sync_from(const godot::Dictionary &data);

  // ------------------------------------------------------------------
  // シリアライズ (SaveService 連携)
  // ------------------------------------------------------------------

  godot::Dictionary serialize() const;
  void deserialize(const godot::Dictionary &data);

  // ------------------------------------------------------------------
  // Signals
  // ------------------------------------------------------------------
  // flag_changed(name: String, value: Variant)  → フラグ変更時
};

} // namespace karakuri

#endif // KARAKURI_FLAG_SERVICE_H
