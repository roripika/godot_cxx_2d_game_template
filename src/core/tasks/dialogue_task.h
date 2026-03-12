#ifndef KARAKURI_DIALOGUE_TASK_H
#define KARAKURI_DIALOGUE_TASK_H

#include "task_base.h"
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

class ScenarioRunner;

class DialogueTask : public TaskBase {
  GDCLASS(DialogueTask, TaskBase)

  godot::String speaker_;
  godot::String text_;
  bool started_ = false;
  
  // 実行時コンテキスト
  ScenarioRunner *runner_ = nullptr;

protected:
  static void _bind_methods();

public:
  DialogueTask() = default;
  ~DialogueTask() override = default;

  void set_runner(ScenarioRunner *runner) override { runner_ = runner; }

  TaskResult execute(double delta) override;
  godot::Error validate_and_setup(const godot::Dictionary &spec) override;
  void complete_instantly() override;

  void set_speaker(const godot::String &speaker) { speaker_ = speaker; }
  godot::String get_speaker() const { return speaker_; }
  
  void set_text(const godot::String &text) { text_ = text; }
  godot::String get_text() const { return text_; }
};

} // namespace karakuri

#endif // KARAKURI_DIALOGUE_TASK_H
