#ifndef KARAKURI_TASK_TEMPLATE_H
#define KARAKURI_TASK_TEMPLATE_H

#include "task_base.h"

namespace karakuri {

/**
 * @brief [Action Name] アクションを実装するタスクのテンプレート。
 *
 * ## AI 開発ルール
 * 1. 新規タスク作成時、必ずこの構造をコピーして使用すること。
 * 2. execute() 内で直接 delta を使用せず、KernelClock を使用すること。
 * 3. UI 参照は直接持たず、シグナルを emit すること。
 */
class TaskTemplate : public TaskBase {
    GDCLASS(TaskTemplate, TaskBase)

private:
    // パラメータ
    godot::String some_param_;
    double duration_ = 0.0;
    
    // 内部状態
    double elapsed_ = 0.0;
    bool finished_ = false;

protected:
    static void _bind_methods();

public:
    TaskTemplate() = default;
    ~TaskTemplate() override = default;

    /**
     * @brief シナリオロード時に一回だけ呼ばれるバリデーションとセットアップ。
     * payload から必要なデータを取り出し、型チェックを行う。
     */
    godot::Error validate_and_setup(const TaskSpec &spec) override;

    /**
     * @brief 毎フレーム呼ばれる実行ロジック。
     * Success を返すと次のアクションへ進む。
     */
    TaskResult execute() override;

    /**
     * @brief [オプション] シナリオを「スキップ」や「一瞬で完了」させる必要がある場合に実装。
     */
    void complete_instantly() override;
};

} // namespace karakuri

#endif // KARAKURI_TASK_TEMPLATE_H
