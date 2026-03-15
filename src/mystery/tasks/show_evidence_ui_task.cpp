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

  ClassDB::bind_method(D_METHOD("get_evidence_presenter_path"),
                       &ShowEvidenceUITask::get_evidence_presenter_path);
  ClassDB::bind_method(D_METHOD("get_target_statement_id"),
                       &ShowEvidenceUITask::get_target_statement_id);
  ClassDB::bind_method(D_METHOD("get_candidate_ids"),
                       &ShowEvidenceUITask::get_candidate_ids);
  ClassDB::bind_method(D_METHOD("get_result_correct"),
                       &ShowEvidenceUITask::get_result_correct);
  ClassDB::bind_method(D_METHOD("get_result_selected_id"),
                       &ShowEvidenceUITask::get_result_selected_id);

  ADD_SIGNAL(MethodInfo("selection_result",
                        PropertyInfo(Variant::BOOL, "is_correct"),
                        PropertyInfo(Variant::STRING, "selected_id")));
}

// ------------------------------------------------------------------
// ライフサイクル
// ------------------------------------------------------------------

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

karakuri::TaskResult ShowEvidenceUITask::execute() {
  if (!started_) {
    result_correct_ = false;
    result_selected_id_ = "";
    // Setup logic normally happens via GDScript or SequencePlayer.
    // ABI v1 assumes the task is ready or setup during validate_and_setup.
    started_ = true;
    return karakuri::TaskResult::Waiting;
  }

  if (finished_) {
    return karakuri::TaskResult::Success;
  }

  return karakuri::TaskResult::Waiting;
}

godot::Error ShowEvidenceUITask::validate_and_setup(const karakuri::TaskSpec &spec) {
  ShowEvidenceUITaskSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("target_statement_id")) {
    ts.target_statement_id = payload["target_statement_id"];
  }
  if (payload.has("candidate_ids")) {
    ts.candidate_ids = payload["candidate_ids"];
  }
  if (payload.has("evidence_presenter_path")) {
    ts.evidence_presenter_path = payload["evidence_presenter_path"];
  }
  
  target_statement_id_ = ts.target_statement_id;
  candidate_ids_ = ts.candidate_ids;
  evidence_presenter_path_ = ts.evidence_presenter_path;
  return godot::OK;
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

NodePath ShowEvidenceUITask::get_evidence_presenter_path() const {
  return evidence_presenter_path_;
}

String ShowEvidenceUITask::get_target_statement_id() const {
  return target_statement_id_;
}

Array ShowEvidenceUITask::get_candidate_ids() const {
  return candidate_ids_;
}

bool ShowEvidenceUITask::get_result_correct() const { return result_correct_; }
String ShowEvidenceUITask::get_result_selected_id() const {
  return result_selected_id_;
}

} // namespace mystery
