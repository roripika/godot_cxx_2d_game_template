#ifndef KARAKURI_GAMES_MYSTERY_TEST_DISCOVER_EVIDENCE_TASK_H
#define KARAKURI_GAMES_MYSTERY_TEST_DISCOVER_EVIDENCE_TASK_H

#include "core/tasks/task_base.h"

namespace karakuri::games::mystery_test {

// Validates payload fields "evidence_id" (or legacy "id") and "location",
// then writes mystery_test:evidence:<id> = true into WorldState SESSION scope.
// Skips the write if the key already exists to prevent duplicate mutations.
class DiscoverEvidenceTask : public TaskBase {
    GDCLASS(DiscoverEvidenceTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::String evidence_id_;
    godot::String location_;
    ScenarioRunner *runner_ = nullptr;

public:
    DiscoverEvidenceTask() = default;
    ~DiscoverEvidenceTask() override = default;

    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
    void set_runner(ScenarioRunner *runner) override { runner_ = runner; }
};

} // namespace karakuri::games::mystery_test

#endif // KARAKURI_GAMES_MYSTERY_TEST_DISCOVER_EVIDENCE_TASK_H
