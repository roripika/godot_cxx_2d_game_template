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

karakuri::TaskResult TaskShowPortrait::execute() {
  if (!started_) {
    MysteryGameState *mgs = MysteryGameState::get_singleton();
    if (mgs) {
      mgs->request_portrait(character_id_, emotion_);
    }
    started_ = true;
  }
  return karakuri::TaskResult::Success;
}

godot::Error TaskShowPortrait::validate_and_setup(const karakuri::TaskSpec &spec) {
  TaskShowPortraitSpec ts;
  const godot::Dictionary &payload = spec.payload;

  if (payload.has("character_id")) {
    ts.character_id = payload["character_id"];
  } else if (payload.has("id")) {
    ts.character_id = payload["id"];
  } else if (payload.has("value")) {
    ts.character_id = payload["value"];
  } else {
    return godot::ERR_INVALID_DATA;
  }

  if (payload.has("emotion")) {
    ts.emotion = payload["emotion"];
  } else {
    ts.emotion = "default";
  }
  
  character_id_ = ts.character_id;
  emotion_ = ts.emotion;
  return godot::OK;
}

void TaskShowPortrait::complete_instantly() {
  execute();
}

String TaskShowPortrait::get_character_id() const { return character_id_; }

String TaskShowPortrait::get_emotion() const { return emotion_; }

} // namespace mystery
