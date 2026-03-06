#ifndef MYSTERY_EVIDENCE_H
#define MYSTERY_EVIDENCE_H

/**
 * @file evidence.h
 * @brief Mystery Layer: ミステリーゲーム固有の証拠品クラス。
 *
 * ## 役割
 * - karakuri::GameItem を継承し、证拠品に特有のフィールドを追加する。
 * - 矛盾可能な証言 ID (contradicts_with_) を保持し、プレゼントロジックを提供する。
 * - is_key_item フラグで重要証拠と一般証拠を区別する。
 *
 * ## 使い方
 * ```gdscript
 * var ev = Evidence.new()
 * ev.item_id = "blood_knife"
 * ev.contradicts_with = "witness_alibi_01"
 * ev.is_key_item = true
 * if ev.can_contradict("witness_alibi_01"):
 *     print("矛盾成立！")
 * ```
 *
 * ## 設計ポリシー
 * - karakuri 層 (GameItem) を拡張するが、
 *   mystery 層は ItemService のカタログにも対応する。
 * - item_id は ItemService カタログの id と一致させること。
 */

#include "../core/items/game_item.h"

#include <godot_cpp/variant/string.hpp>

namespace mystery {

class Evidence : public karakuri::GameItem {
  GDCLASS(Evidence, karakuri::GameItem)

private:
  /// @brief ItemService マスターカタログの id と対応するキー。
  godot::String item_id_;

  /// @brief この証拠が矛盾させられる証言・シーンの ID。
  godot::String contradicts_with_;

  /// @brief 重要証拠フラグ（破棄不可、UI で強調など）。
  bool is_key_item_ = false;

  static void _bind_methods();

public:
  Evidence() = default;
  ~Evidence() override = default;

  // ------------------------------------------------------------------
  // プロパティ
  // ------------------------------------------------------------------

  void set_item_id(const godot::String &id);
  godot::String get_item_id() const;

  void set_contradicts_with(const godot::String &statement_id);
  godot::String get_contradicts_with() const;

  void set_is_key_item(bool value);
  bool get_is_key_item() const;

  // ------------------------------------------------------------------
  // ゲームロジック
  // ------------------------------------------------------------------

  /**
   * @brief この証拠品が指定された証言に矛盾できるか確認する。
   * @param statement_id 証言・シーン識別子。
   * @return contradicts_with_ と一致すれば true。
   */
  bool can_contradict(const godot::String &statement_id) const;
};

} // namespace mystery

#endif // MYSTERY_EVIDENCE_H
