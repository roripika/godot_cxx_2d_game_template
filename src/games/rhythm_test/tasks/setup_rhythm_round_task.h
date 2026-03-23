#pragma once

#include "core/tasks/task_base.h"

namespace karakuri::games::rhythm_test {

class SetupRhythmRoundTask : public TaskBase {
    GDCLASS(SetupRhythmRoundTask, TaskBase)

protected:
    static void _bind_methods();

private:
    godot::Array notes_;
    godot::Array taps_;
    int advance_ms_ = 1000;
    int perfect_window_ms_ = 50;
    int good_window_ms_ = 150;
    int clear_hit_count_ = 3;
    int max_miss_count_ = 1;

public:
    godot::Error validate_and_setup(const TaskSpec &spec) override;
    TaskResult execute() override;
    void complete_instantly() override {}
};

} // namespace karakuri::games::rhythm_test