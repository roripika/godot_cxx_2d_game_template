#ifndef KARAKURI_MYSTERY_TEST_CHECK_EVIDENCE_TASK_H
#define KARAKURI_MYSTERY_TEST_CHECK_EVIDENCE_TASK_H

#include "core/tasks/task_base.h"
#include "core/scenario/scenario_runner.h"

namespace karakuri {

class CheckEvidenceTask : public TaskBase {
    GDCLASS(CheckEvidenceTask, TaskBase)

private:
    godot::String evidence_id_;
    godot::String if_true_;
    godot::String if_false_;
    ScenarioRunner *runner_ = nullptr; // Added runner pointer

protected:
    static void _bind_methods();

public:
    CheckEvidenceTask() = default;
    ~CheckEvidenceTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri

#endif // KARAKURI_MYSTERY_TEST_CHECK_EVIDENCE_TASK_H
