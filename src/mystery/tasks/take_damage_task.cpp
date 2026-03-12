#include "take_damage_task.h"
#include "../mystery_game_state.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace mystery {

karakuri::TaskResult TakeDamageTask::execute(double /*delta*/) {
  MysteryGameState *mgs = MysteryGameState::get_singleton();
  if (mgs) {
    for (int i = 0; i < amount_; ++i) mgs->take_damage();
  }
  return karakuri::TaskResult::Success;
}

Error TakeDamageTask::validate_and_setup(const Dictionary &spec) {
  if (spec.has("value")) amount_ = (int)spec["value"];
  else if (spec.has("amount")) amount_ = (int)spec["amount"];
  return OK;
}

void TakeDamageTask::complete_instantly() {
  execute(0.0);
}

} // namespace mystery
