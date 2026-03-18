#include "wait_for_signal_task.h"

#include "core/scenario/scenario_runner.h"
#include "core/kernel_clock.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri::games::mystery_test {

void WaitForSignalTask::_bind_methods() {}

godot::Error WaitForSignalTask::validate_and_setup(const TaskSpec &spec) {
    if (!spec.payload.has("signal")) {
        godot::UtilityFunctions::push_error("[WaitForSignalTask] 'signal' is missing in payload.");
        return godot::ERR_INVALID_DATA;
    }
    signal_name_ = spec.payload["signal"];
    timeout_ = spec.payload.get("timeout", 5.0);
    start_time_ = KernelClock::get_singleton()->now();
    return godot::OK;
}

TaskResult WaitForSignalTask::execute() {
    double now = KernelClock::get_singleton()->now();
    
    // タイムアウトチェック
    if (now - start_time_ >= timeout_) {
        godot::UtilityFunctions::push_warning("[WaitForSignalTask] Timeout exceeded for signal: ", signal_name_);
        return TaskResult::Failed;
    }

    // シナリオランナーの待機フラグを確認 (ShowDialogueTaskと同様の仕組みを利用)
    // 本来はシグナル受信をフックすべきだが、最小実装テストとして
    // runner->is_waiting() や外部からの通知を待機する設計にする。
    // 今回は KernelClock の決定論的タイマーテストを優先。

    return TaskResult::Waiting;
}

} // namespace karakuri::games::mystery_test
