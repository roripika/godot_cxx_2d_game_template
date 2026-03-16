#include "task_template.h"

#include "../scenario/scenario_runner.h"
#include "../kernel_clock.h"
#include <godot_cpp/variant/utility_functions.hpp>

namespace karakuri {

void TaskTemplate::_bind_methods() {
    // 必要に応じてリフレクション情報を登録（基本は不要）
}

godot::Error TaskTemplate::validate_and_setup(const TaskSpec &spec) {
    // 1. 必須パラメータの存在確認
    if (!spec.payload.has("some_param")) {
        godot::UtilityFunctions::push_error("[TaskTemplate] 'some_param' が指定されていません。");
        return godot::ERR_INVALID_PARAMETER;
    }

    // 2. 型付きでの取得
    some_param_ = spec.payload["some_param"];
    duration_ = spec.payload.get("duration", 1.0); // デフォルト値付き

    return godot::OK;
}

TaskResult TaskTemplate::execute() {
    // 決定論的な現在時刻を取得
    // double now = KernelClock::get_singleton()->now();

    // 単純なタイマー待ちの例
    // ※ 実際は elapsed_ += delta ではなく、終了時刻との比較が望ましい
    // if (elapsed_ < duration_) {
    //     return TaskResult::Running;
    // }

    godot::UtilityFunctions::print("[TaskTemplate] executed with param: ", some_param_);

    return TaskResult::Success;
}

void TaskTemplate::complete_instantly() {
    finished_ = true;
}

} // namespace karakuri
