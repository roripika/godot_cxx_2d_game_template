#include "task_change_background.h"
#include "../mystery_game_state.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace mystery {

void TaskChangeBackground::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_background_id", "id"),
                       &TaskChangeBackground::set_background_id);
  ClassDB::bind_method(D_METHOD("get_background_id"),
                       &TaskChangeBackground::get_background_id);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "background_id"),
               "set_background_id", "get_background_id");
}

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

karakuri::TaskResult TaskChangeBackground::execute(double /*delta*/) {
  if (!started_) {
    MysteryGameState *mgs = MysteryGameState::get_singleton();
    if (mgs) {
      mgs->request_background(background_id_);
    }
    started_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error TaskChangeBackground::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("id")) {
    background_id_ = spec["id"];
  } else if (spec.has("value")) {
    background_id_ = spec["value"];
  } else {
    return godot::ERR_INVALID_DATA;
  }
  return godot::OK;
}

void TaskChangeBackground::complete_instantly() {
  execute(0.0);
}

void TaskChangeBackground::set_background_id(const String &id) {
  background_id_ = id;
}
String TaskChangeBackground::get_background_id() const {
  return background_id_;
}

} // namespace mystery
