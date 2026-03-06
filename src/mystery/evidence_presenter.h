#ifndef MYSTERY_EVIDENCE_PRESENTER_H
#define MYSTERY_EVIDENCE_PRESENTER_H

/**
 * @file evidence_presenter.h
 * @brief Mystery Layer: 証拠品選択ロジック管理。
 *
 * ## 役割
 * - 証拠品選択 UI（EvidenceUI）の表示・非表示を制御する。
 * - InputService / ItemService と連携してプレイヤーの所持品リストを表示する。
 * - プレイヤーが選択した証拠品が `Evidence::can_contradict(target_id)` と一致するか判定。
 * - 結果を `selection_finished(is_correct, selected_id)` シグナルで通知。
 *
 * ## ScenarioRunner との連携
 * - present_evidence アクションから start_selection() を呼ぶ。
 * - selection_finished シグナルを ScenarioRunner の complete_custom_action() に繋ぐ。
 *
 * ## Godot シーンでの使い方
 * 1. EvidencePresenter ノードを ScenarioRunner の兄弟もしくは子に配置する。
 * 2. インスペクタで evidence_ui_path (EvidenceUI ノード) を設定する。
 * 3. ScenarioRunner の present_evidence アクションが自動的に呼び出す。
 */

#include "../core/services/item_service.h"
#include "evidence.h"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>

namespace mystery {

class EvidencePresenter : public godot::Node {
  GDCLASS(EvidencePresenter, godot::Node)

private:
  /// @brief 証拠品選択 UI ノードへのパス（EvidenceUI / InventoryUI 等を想定）
  godot::NodePath evidence_ui_path_;

  /// @brief 現在突きつけ中の証言・シーン識別子
  godot::String target_statement_id_;

  /// @brief 選択受付中フラグ
  bool selecting_ = false;

  static void _bind_methods();

  godot::Node *get_evidence_ui() const;

public:
  EvidencePresenter() = default;
  ~EvidencePresenter() override = default;

  // ------------------------------------------------------------------
  // 証拠品選択 API
  // ------------------------------------------------------------------

  /**
   * @brief 証拠品選択を開始する（ScenarioRunner から呼ばれる）。
   * @param target_statement_id  突きつけ対象の証言・シーン ID。
   * @param candidate_ids        表示する証拠品 ID の配列。空なら所持品全件。
   */
  void start_selection(const godot::String &target_statement_id,
                       const godot::Array &candidate_ids);

  /**
   * @brief EvidenceUI の選択シグナルハンドラ（UI 側から呼ばれる）。
   * @param selected_id  プレイヤーが選んだアイテム ID。
   */
  void on_evidence_selected(const godot::String &selected_id);

  /**
   * @brief 選択をキャンセルする（開発便宜用）。
   */
  void cancel_selection();

  bool is_selecting() const { return selecting_; }

  // ------------------------------------------------------------------
  // プロパティ
  // ------------------------------------------------------------------
  void set_evidence_ui_path(const godot::NodePath &path);
  godot::NodePath get_evidence_ui_path() const;

  // ------------------------------------------------------------------
  // Signals
  // ------------------------------------------------------------------
  // selection_finished(is_correct: bool, selected_id: String)
  //   → 正解・不正解どちらでも発火。呼び出し元が分岐処理する。
};

} // namespace mystery

#endif // MYSTERY_EVIDENCE_PRESENTER_H
