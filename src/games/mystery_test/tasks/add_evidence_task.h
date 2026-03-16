#ifndef KARAKURI_MYSTERY_TEST_ADD_EVIDENCE_TASK_H
#define KARAKURI_MYSTERY_TEST_ADD_EVIDENCE_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri {

class AddEvidenceTask : public TaskBase {
    GDCLASS(AddEvidenceTask, TaskBase)

private:
    godot::String evidence_id_;
    ScenarioRunner *runner_ = nullptr;

protected:
    static void _bind_methods();

public:
    AddEvidenceTask() = default;
    ~AddEvidenceTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri

#endif // KARAKURI_MYSTERY_TEST_ADD_EVIDENCE_TASK_H
