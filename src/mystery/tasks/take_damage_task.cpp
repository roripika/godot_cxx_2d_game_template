#include "take_damage_task.h"
#include "../mystery_game_state.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace mystery {

karakuri::TaskResult TakeDamageTask::execute() {
  MysteryGameState *mgs = MysteryGameState::get_singleton();
  if (mgs) {
    for (int i = 0; i < amount_; ++i) mgs->take_damage();
  }
  return karakuri::TaskResult::Success;
}

godot::Error TakeDamageTask::validate_and_setup(const karakuri::TaskSpec &spec) {
  TakeDamageTaskSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("value")) ts.amount = (int)payload["value"];
  else if (payload.has("amount")) ts.amount = (int)payload["amount"];

  amount_ = ts.amount;
  return godot::OK;
}

void TakeDamageTask::complete_instantly() {
  execute();
}

} // namespace mystery
