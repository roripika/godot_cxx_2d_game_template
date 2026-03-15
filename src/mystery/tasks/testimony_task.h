#ifndef MYSTERY_TESTIMONY_TASK_H
#define MYSTERY_TESTIMONY_TASK_H

#include "../../core/tasks/task_base.h"
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include "../../core/tasks/task_spec.h"

namespace karakuri { class ScenarioRunner; }

namespace mystery {

struct TestimonyTaskSpec {
  godot::Array lines;
  godot::Array on_success;
  godot::Array on_failure;
  int max_rounds = 3;
};


class TestimonyTask : public karakuri::TaskBase {
  GDCLASS(TestimonyTask, karakuri::TaskBase)

  godot::Array lines_;
  godot::Array on_success_;
  godot::Array on_failure_;
  int max_rounds_ = 3;
  
  karakuri::ScenarioRunner *runner_ = nullptr;
  bool started_ = false;

protected:
  static void _bind_methods() {}

public:
  TestimonyTask() = default;
  ~TestimonyTask() override = default;

  void set_runner(karakuri::ScenarioRunner *runner) override { runner_ = runner; }

  karakuri::TaskResult execute() override;
  godot::Error validate_and_setup(const karakuri::TaskSpec &spec) override;
  void complete_instantly() override;
};

} // namespace mystery

#endif // MYSTERY_TESTIMONY_TASK_H
