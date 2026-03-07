#include "show_evidence_ui_task.h"

#include "../evidence_presenter.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void ShowEvidenceUITask::_bind_methods() {
  ClassDB::bind_method(D_METHOD("_on_selection_finished", "is_correct", "selected_id"),
                       &ShowEvidenceUITask::_on_selection_finished);

  ClassDB::bind_method(D_METHOD("set_evidence_presenter_path", "path"),
                       &ShowEvidenceUITask::set_evidence_presenter_path);
  ClassDB::bind_method(D_METHOD("get_evidence_presenter_path"),
                       &ShowEvidenceUITask::get_evidence_presenter_path);
  ClassDB::bind_method(D_METHOD("set_target_statement_id", "id"),
                       &ShowEvidenceUITask::set_target_statement_id);
  ClassDB::bind_method(D_METHOD("get_target_statement_id"),
                       &ShowEvidenceUITask::get_target_statement_id);
  ClassDB::bind_method(D_METHOD("set_candidate_ids", "ids"),
                       &ShowEvidenceUITask::set_candidate_ids);
  ClassDB::bind_method(D_METHOD("get_candidate_ids"),
                       &ShowEvidenceUITask::get_candidate_ids);
  ClassDB::bind_method(D_METHOD("get_result_correct"),
                       &ShowEvidenceUITask::get_result_correct);
  ClassDB::bind_method(D_METHOD("get_result_selected_id"),
                       &ShowEvidenceUITask::get_result_selected_id);

  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "evidence_presenter_path"),
               "set_evidence_presenter_path", "get_evidence_presenter_path");
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "target_statement_id"),
               "set_target_statement_id", "get_target_statement_id");

  ADD_SIGNAL(MethodInfo("selection_result",
                        PropertyInfo(Variant::BOOL, "is_correct"),
                        PropertyInfo(Variant::STRING, "selected_id")));
}

// ------------------------------------------------------------------
// ライフサイクル
// ------------------------------------------------------------------

void ShowEvidenceUITask::on_start() {
  finished_ = false;
  result_correct_ = false;
  result_selected_id_ = "";

  // Note: ShowEvidenceUITask は RefCounted なのでノードツリーに参照を持てない。
  // EvidencePresenter へのアクセスは SequencePlayer または呼び出し元が
  // 外部から _on_selection_finished() を直接呼ぶことで完了通知を受け取る設計。
  // GDScript 側から evidence_presenter.connect("selection_finished", task._on_selection_finished)
  // のように接続することを推奨する。
  //
  // C++ からは SequencePlayer が NodePath 経由で EvidencePresenter を取得し、
  // start_selection() を呼ぶとともにシグナルをこのタスクのメソッドに接続する。
}

void ShowEvidenceUITask::complete_instantly() {
  // UIをキャンセルして即完了とする
  result_correct_ = false;
  result_selected_id_ = "";
  finished_ = true;
  emit_signal("selection_result", false, String(""));
}

// ------------------------------------------------------------------
// シグナルハンドラ
// ------------------------------------------------------------------

void ShowEvidenceUITask::_on_selection_finished(bool is_correct,
                                                 const String &selected_id) {
  result_correct_ = is_correct;
  result_selected_id_ = selected_id;
  finished_ = true;
  emit_signal("selection_result", is_correct, selected_id);
}

// ------------------------------------------------------------------
// プロパティ
// ------------------------------------------------------------------

void ShowEvidenceUITask::set_evidence_presenter_path(const NodePath &path) {
  evidence_presenter_path_ = path;
}
NodePath ShowEvidenceUITask::get_evidence_presenter_path() const {
  return evidence_presenter_path_;
}

void ShowEvidenceUITask::set_target_statement_id(const String &id) {
  target_statement_id_ = id;
}
String ShowEvidenceUITask::get_target_statement_id() const {
  return target_statement_id_;
}

void ShowEvidenceUITask::set_candidate_ids(const Array &ids) {
  candidate_ids_ = ids;
}
Array ShowEvidenceUITask::get_candidate_ids() const {
  return candidate_ids_;
}

bool ShowEvidenceUITask::get_result_correct() const { return result_correct_; }
String ShowEvidenceUITask::get_result_selected_id() const {
  return result_selected_id_;
}

} // namespace mystery
