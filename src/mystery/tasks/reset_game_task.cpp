#include "reset_game_task.h"
#include "../mystery_game_state.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace mystery {

karakuri::TaskResult ResetGameTask::execute() {
  MysteryGameState *mgs = MysteryGameState::get_singleton();
  if (mgs) mgs->reset_game();
  return karakuri::TaskResult::Success;
}

godot::Error ResetGameTask::validate_and_setup(const karakuri::TaskSpec &spec) {
  return godot::OK;
}

void ResetGameTask::complete_instantly() {
  execute();
}

} // namespace mystery
