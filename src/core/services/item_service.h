#ifndef KARAKURI_ITEM_SERVICE_H
#define KARAKURI_ITEM_SERVICE_H

/**
 * @file item_service.h
 * @brief Basic Game Karakuri: 汎用アイテム管理サービス。
 *
 * ## 役割
 * - アイテムマスターカタログ（JSON からロード可能）の管理。
 * - プレイヤーの現在所持アイテム（インベントリ）の管理。
 * - SaveService と連携した所持状態の保存・復元。
 *
 * ## 設計ポリシー (karakuri 層)
 * - ミステリー固有の概念（証拠品・証言等）を一切知らない。
 * - mystery 層は ItemService を利用して「証拠品フィルタリング」等を行う。
 *
 * ## 使い方
 * ```cpp
 * auto *svc = karakuri::ItemService::get_singleton();
 * svc->register_item("knife", { "name": "血のナイフ", ... });
 * svc->add_item("knife");
 * bool has = svc->has_item("knife");  // true
 * ```
 *
 * ## マスターカタログの Dictionary 形式
 * ```json
 * {
 *   "id": "knife",
 *   "name": "血のついたナイフ",
 *   "description": "被害者の部屋で発見。",
 *   "icon_path": "res://assets/items/knife.png",
 *   "metadata": {}
 * }
 * ```
 */

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class ItemService : public godot::Node {
  GDCLASS(ItemService, godot::Node)

private:
  static ItemService *singleton_;

  /// @brief id → Dictionary（name / description / icon_path / metadata）
  godot::Dictionary item_master_;

  /// @brief 現在所持しているアイテム ID の配列
  godot::Array inventory_;

  static void _bind_methods();

public:
  ItemService();
  ~ItemService() override;

  static ItemService *get_singleton();

  // ------------------------------------------------------------------
  // マスターカタログ
  // ------------------------------------------------------------------

  /**
   * @brief JSON ファイルからマスターカタログを一括ロードする。
   * @param path res:// パス。配列形式の JSON を期待する。
   */
  void load_master_data(const godot::String &path);

  /**
   * @brief アイテムを1件マスターに登録する（手動登録用）。
   * @param id   アイテム識別子。
   * @param data {name, description, icon_path, metadata} の Dictionary。
   */
  void register_item(const godot::String &id, const godot::Dictionary &data);

  /**
   * @brief マスターからアイテムデータを取得する。
   * @return 見つからない場合は空 Dictionary。
   */
  godot::Dictionary get_item_data(const godot::String &id) const;

  /**
   * @brief マスターにアイテムが登録されているか確認する。
   */
  bool is_known_item(const godot::String &id) const;

  // ------------------------------------------------------------------
  // インベントリ
  // ------------------------------------------------------------------

  /**
   * @brief アイテムをインベントリに追加する（重複追加不可）。
   * @return 追加されたら true（既に所持中なら false）。
   */
  bool add_item(const godot::String &id);

  /**
   * @brief アイテムをインベントリから削除する。
   * @return 削除されたら true。
   */
  bool remove_item(const godot::String &id);

  /**
   * @brief アイテムを所持しているか確認する。
   */
  bool has_item(const godot::String &id) const;

  /**
   * @brief 所持アイテムの ID 配列を返す。
   */
  godot::Array get_inventory() const;

  /**
   * @brief 所持アイテムのデータ配列を返す（UI 表示用）。
   * @return [{ id, name, description, icon_path, metadata }, ...] の Array。
   */
  godot::Array get_inventory_data() const;

  /**
   * @brief インベントリを全消去する。
   */
  void clear_inventory();

  // ------------------------------------------------------------------
  // シリアライズ (SaveService 連携)
  // ------------------------------------------------------------------

  /**
   * @brief インベントリを Dictionary に変換する（保存用）。
   * @return {"inventory": ["id1", "id2", ...]}
   */
  godot::Dictionary serialize() const;

  /**
   * @brief Dictionary からインベントリを復元する（読み込み用）。
   */
  void deserialize(const godot::Dictionary &data);

  // ------------------------------------------------------------------
  // Signals
  // ------------------------------------------------------------------
  // item_added(id: String)    → アイテムが追加されたとき（UI 更新トリガー）
  // item_removed(id: String)  → アイテムが削除されたとき
};

} // namespace karakuri

#endif // KARAKURI_ITEM_SERVICE_H
