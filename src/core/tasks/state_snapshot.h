#ifndef KARAKURI_STATE_SNAPSHOT_H
#define KARAKURI_STATE_SNAPSHOT_H

/**
 * @file state_snapshot.h
 * @brief Basic Game Karakuri: ゲーム状態のスナップショット（フラグ・インベントリ・シーン位置）。
 *
 * ## 役割
 * - SequencePlayer が rollback 用に任意タイミングで状態を保存する。
 * - capture(scene_id, action_index) で現在の FlagService + ItemService の内容を
 *   ディープコピーして保持する。
 * - restore() で FlagService・ItemService を保存時の状態に戻す。
 *   ScenarioRunner への位置復元は SequencePlayer が担当する。
 *
 * ## duplicate(true) ポリシー
 * Dictionary::duplicate(true) / Array::duplicate(true) を用いてネストも含めた
 * ディープコピーを行う。シャローコピーでは復元時に意図しない変更が混入するため注意。
 */

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class StateSnapshot : public godot::Resource {
  GDCLASS(StateSnapshot, godot::Resource)

  godot::Dictionary flags_;           ///< FlagService::serialize() のディープコピー
  godot::Array inventory_;            ///< ItemService::get_inventory() のディープコピー
  godot::String current_scene_id_;    ///< ScenarioRunner::get_current_scene_id() の値
  int current_command_index_ = 0;     ///< ScenarioRunner の pending_action_index_ の値

protected:
  static void _bind_methods();

public:
  StateSnapshot() = default;
  ~StateSnapshot() override = default;

  // ------------------------------------------------------------------
  // ファクトリ
  // ------------------------------------------------------------------

  /**
   * @brief 現在の FlagService + ItemService の状態と、呼び出し元が渡したシーン位置を
   *        キャプチャして新しい StateSnapshot を返す。
   *
   * @param scene_id        ScenarioRunner::get_current_scene_id() の戻り値
   * @param action_index    ScenarioRunner::get_pending_action_index() の戻り値
   * @return Ref<StateSnapshot>
   */
  static godot::Ref<StateSnapshot> capture(const godot::String &scene_id,
                                           int action_index);

  // ------------------------------------------------------------------
  // 復元
  // ------------------------------------------------------------------

  /**
   * @brief FlagService と ItemService をスナップショット時の状態に戻す。
   * ScenarioRunner への復元は呼び出し元（SequencePlayer）の責任。
   */
  void restore_services() const;

  // ------------------------------------------------------------------
  // ゲッター
  // ------------------------------------------------------------------
  godot::String get_scene_id() const;
  int get_command_index() const;
  godot::Dictionary get_flags() const;
  godot::Array get_inventory() const;

  // ------------------------------------------------------------------
  // セッター（エディタ用・GDScript からの手動設定用）
  // ------------------------------------------------------------------
  void set_scene_id(const godot::String &id);
  void set_command_index(int index);
};

} // namespace karakuri

#endif // KARAKURI_STATE_SNAPSHOT_H
