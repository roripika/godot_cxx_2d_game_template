#include "evidence_presenter.h"

#include "../core/services/item_service.h"
#include "evidence.h"

#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void EvidencePresenter::_bind_methods() {
  ClassDB::bind_method(
      D_METHOD("start_selection", "target_statement_id", "candidate_ids"),
      &EvidencePresenter::start_selection);
  ClassDB::bind_method(D_METHOD("on_evidence_selected", "selected_id"),
                       &EvidencePresenter::on_evidence_selected);
  ClassDB::bind_method(D_METHOD("cancel_selection"),
                       &EvidencePresenter::cancel_selection);
  ClassDB::bind_method(D_METHOD("is_selecting"), &EvidencePresenter::is_selecting);

  ClassDB::bind_method(D_METHOD("set_evidence_ui_path", "path"),
                       &EvidencePresenter::set_evidence_ui_path);
  ClassDB::bind_method(D_METHOD("get_evidence_ui_path"),
                       &EvidencePresenter::get_evidence_ui_path);
  ADD_PROPERTY(
      PropertyInfo(Variant::NODE_PATH, "evidence_ui_path"),
      "set_evidence_ui_path", "get_evidence_ui_path");

  ADD_SIGNAL(MethodInfo("selection_finished",
                        PropertyInfo(Variant::BOOL, "is_correct"),
                        PropertyInfo(Variant::STRING, "selected_id")));
}

// ---------------------------------------------------------------------------
// UI ヘルパー
// ---------------------------------------------------------------------------
Node *EvidencePresenter::get_evidence_ui() const {
  if (evidence_ui_path_.is_empty())
    return nullptr;
  return get_node_or_null(evidence_ui_path_);
}

// ---------------------------------------------------------------------------
// 証拠品選択開始
// ---------------------------------------------------------------------------
void EvidencePresenter::start_selection(const String &target_statement_id,
                                        const Array &candidate_ids) {
  target_statement_id_ = target_statement_id;
  selecting_ = true;

  // UI に表示する候補リストを構築
  Array display_ids;
  if (candidate_ids.is_empty()) {
    // 指定なし → 所持品全件
    auto *svc = karakuri::ItemService::get_singleton();
    if (svc) {
      display_ids = svc->get_inventory();
    }
  } else {
    display_ids = candidate_ids;
  }

  // EvidenceUI の表示（シグナル接続済みを前提）
  Node *ui = get_evidence_ui();
  CanvasItem *ui_ci = Object::cast_to<CanvasItem>(ui);
  if (ui) {
    if (ui_ci) ui_ci->set_visible(true);
    // UI が "show_items(ids: Array)" メソッドを持っていれば呼ぶ
    if (ui->has_method("show_items")) {
      ui->call("show_items", display_ids);
    }
    // UI の evidence_selected シグナルをまだ接続していなければ接続
    if (ui->has_signal("evidence_selected") &&
        !ui->is_connected("evidence_selected",
                          Callable(this, "on_evidence_selected"))) {
      ui->connect("evidence_selected",
                  Callable(this, "on_evidence_selected"));
    }
  } else {
    // UIが設定されていない → 開発時のフォールバック: 即時「不正解」
    UtilityFunctions::push_warning(
        "EvidencePresenter: evidence_ui_path not set. Auto-resolving as wrong.");
    selecting_ = false;
    emit_signal("selection_finished", false, String(""));
  }
}

// ---------------------------------------------------------------------------
// UI からの選択コールバック
// ---------------------------------------------------------------------------
void EvidencePresenter::on_evidence_selected(const String &selected_id) {
  if (!selecting_)
    return;
  selecting_ = false;

  // UI を非表示に
  Node *ui = get_evidence_ui();
  CanvasItem *ui_ci = Object::cast_to<CanvasItem>(ui);
  if (ui_ci)
    ui_ci->set_visible(false);

  // Evidence リソースを ItemService の is_known_item 経由で取得する。
  // Evidence は GDScript 側でマスターに登録されているケースを想定。
  // まず ItemService に問い合わせ、metadata に "contradicts_with" があれば使う。
  bool is_correct = false;
  auto *svc = karakuri::ItemService::get_singleton();
  if (svc && svc->has_item(selected_id)) {
    Dictionary item_data = svc->get_item_data(selected_id);
    if (item_data.has("metadata")) {
      Dictionary meta = item_data["metadata"];
      if (meta.has("contradicts_with")) {
        String contradicts = meta["contradicts_with"];
        is_correct = (contradicts == target_statement_id_);
      }
    }
  }

  UtilityFunctions::print(
      String("[EvidencePresenter] selected=") + selected_id +
      String(" target=") + target_statement_id_ +
      String(" correct=") + (is_correct ? "true" : "false"));

  emit_signal("selection_finished", is_correct, selected_id);
}

// ---------------------------------------------------------------------------
// キャンセル
// ---------------------------------------------------------------------------
void EvidencePresenter::cancel_selection() {
  if (!selecting_)
    return;
  selecting_ = false;
  Node *ui_c = get_evidence_ui();
  CanvasItem *ci_cancel = Object::cast_to<CanvasItem>(ui_c);
  if (ci_cancel)
    ci_cancel->set_visible(false);
  emit_signal("selection_finished", false, String(""));
}

// ---------------------------------------------------------------------------
// プロパティ
// ---------------------------------------------------------------------------
void EvidencePresenter::set_evidence_ui_path(const NodePath &path) {
  evidence_ui_path_ = path;
}
NodePath EvidencePresenter::get_evidence_ui_path() const {
  return evidence_ui_path_;
}

} // namespace mystery
