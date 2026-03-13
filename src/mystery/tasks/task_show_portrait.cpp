#include "task_show_portrait.h"
#include "../mystery_game_state.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void TaskShowPortrait::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_character_id"),
                       &TaskShowPortrait::get_character_id);
  ClassDB::bind_method(D_METHOD("get_emotion"), &TaskShowPortrait::get_emotion);
}

// ------------------------------------------------------------------
// ライフサイクル (ABI v1)
// ------------------------------------------------------------------

karakuri::TaskResult TaskShowPortrait::execute(double /*delta*/) {
  if (!started_) {
    MysteryGameState *mgs = MysteryGameState::get_singleton();
    if (mgs) {
      mgs->request_portrait(character_id_, emotion_);
    }
    started_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error TaskShowPortrait::validate_and_setup(const godot::Dictionary &spec) {
  if (spec.has("character_id")) {
    character_id_ = spec["character_id"];
  } else if (spec.has("id")) {
    character_id_ = spec["id"];
  } else if (spec.has("value")) {
    character_id_ = spec["value"];
  } else {
    return godot::ERR_INVALID_DATA;
  }

  if (spec.has("emotion")) {
    emotion_ = spec["emotion"];
  }
  
  return godot::OK;
}

void TaskShowPortrait::complete_instantly() {
  execute(0.0);
}

String TaskShowPortrait::get_character_id() const { return character_id_; }

String TaskShowPortrait::get_emotion() const { return emotion_; }

} // namespace mystery
