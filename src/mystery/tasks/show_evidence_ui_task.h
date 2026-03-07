#ifndef MYSTERY_SHOW_EVIDENCE_UI_TASK_H
#define MYSTERY_SHOW_EVIDENCE_UI_TASK_H

/**
 * @file show_evidence_ui_task.h
 * @brief Mystery: EvidencePresenter の表示をタスクとしてラップする。
 *
 * ## 動作
 * - on_start()          : EvidencePresenter::start_selection() を呼び、
 *                         selection_finished シグナルを _on_selection_finished() に接続する。
 * - on_update(delta)    : シグナル待機（何もしない）。
 * - complete_instantly(): UIを非表示にして finished_ = true。
 *
 * ## シグナル
 * selection_result(is_correct: bool, selected_id: String) を外部に再発行する。
 */

#include "../../core/tasks/task_base.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>

namespace mystery {

class ShowEvidenceUITask : public karakuri::TaskBase {
  GDCLASS(ShowEvidenceUITask, karakuri::TaskBase)

  godot::NodePath evidence_presenter_path_;
  godot::String target_statement_id_;
  godot::Array candidate_ids_; ///< String の配列

  bool result_correct_ = false;
  godot::String result_selected_id_;

protected:
  static void _bind_methods();

public:
  ShowEvidenceUITask() = default;
  ~ShowEvidenceUITask() override = default;

  // ------------------------------------------------------------------
  // ライフサイクル
  // ------------------------------------------------------------------
  void on_start() override;
  void complete_instantly() override;

  // ------------------------------------------------------------------
  // シグナルハンドラ（内部から EvidencePresenter のシグナルを受け取る）
  // ------------------------------------------------------------------
  void _on_selection_finished(bool is_correct, const godot::String &selected_id);

  // ------------------------------------------------------------------
  // プロパティ
  // ------------------------------------------------------------------
  void set_evidence_presenter_path(const godot::NodePath &path);
  godot::NodePath get_evidence_presenter_path() const;

  void set_target_statement_id(const godot::String &id);
  godot::String get_target_statement_id() const;

  void set_candidate_ids(const godot::Array &ids);
  godot::Array get_candidate_ids() const;

  bool get_result_correct() const;
  godot::String get_result_selected_id() const;
};

} // namespace mystery

#endif // MYSTERY_SHOW_EVIDENCE_UI_TASK_H
