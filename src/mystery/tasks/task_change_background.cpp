#include "task_change_background.h"
#include "../mystery_game_state.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace mystery {

void TaskChangeBackground::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_background_id"),
                       &TaskChangeBackground::get_background_id);
}

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

karakuri::TaskResult TaskChangeBackground::execute() {
  if (!started_) {
    MysteryGameState *mgs = MysteryGameState::get_singleton();
    if (mgs) {
      mgs->request_background(background_id_);
    }
    started_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error TaskChangeBackground::validate_and_setup(const karakuri::TaskSpec &spec) {
  TaskChangeBackgroundSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("id")) {
    ts.background_id = payload["id"];
  } else if (payload.has("value")) {
    ts.background_id = payload["value"];
  } else {
    return godot::ERR_INVALID_DATA;
  }
  
  background_id_ = ts.background_id;
  return godot::OK;
}

void TaskChangeBackground::complete_instantly() {
  execute();
}

String TaskChangeBackground::get_background_id() const {
  return background_id_;
}

} // namespace mystery
