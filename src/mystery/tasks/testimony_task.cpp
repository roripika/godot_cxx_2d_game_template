#include "testimony_task.h"
#include "../../core/scenario/scenario_runner.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

karakuri::TaskResult TestimonyTask::execute(double /*delta*/) {
  if (runner_ == nullptr) return karakuri::TaskResult::Failed;

  if (!started_) {
    UtilityFunctions::print("[TestimonyTask] Starting testimony flow.");
    
    // 暫定実装: 最初の行を表示して一旦停止（旧 lambda 互換）
    if (!lines_.is_empty()) {
        Dictionary line = lines_[0];
        String speaker = line.has("speaker") ? String(line["speaker"]) : "Witness";
        String text = line.has("text_key") ? String(line["text_key"]) : "...";
        runner_->emit_signal("dialogue_requested", speaker, text);
        runner_->set_waiting_for_dialogue(true);
    }
    
    started_ = true;
  }

  if (runner_->is_waiting_for_dialogue()) {
    return karakuri::TaskResult::Waiting;
  }

  // 本来はここで証言のループや証拠提示待ちを行うが、
  // 現状は ABI v1 への移行が主目的のため、一件進めて成功扱いにする。
  // TODO: 本格的な証言ロジックの実装
  
  return karakuri::TaskResult::Success;
}

Error TestimonyTask::validate_and_setup(const Dictionary &spec) {
  if (spec.has("testimonies")) {
    lines_ = spec["testimonies"];
    on_success_ = spec.has("on_success") ? Array(spec["on_success"]) : Array();
    on_failure_ = spec.has("on_failure") ? Array(spec["on_failure"]) : Array();
    if (spec.has("max_rounds")) max_rounds_ = (int)spec["max_rounds"];
  } else {
    return ERR_INVALID_DATA;
  }
  return OK; 
}

void TestimonyTask::complete_instantly() {
  started_ = true;
}

} // namespace mystery
