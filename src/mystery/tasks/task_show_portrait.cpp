#include "task_show_portrait.h"
#include "../mystery_game_state.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

void TaskShowPortrait::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_character_id", "id"),
                       &TaskShowPortrait::set_character_id);
  ClassDB::bind_method(D_METHOD("get_character_id"),
                       &TaskShowPortrait::get_character_id);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "character_id"),
               "set_character_id", "get_character_id");

  ClassDB::bind_method(D_METHOD("set_emotion", "emotion"),
                       &TaskShowPortrait::set_emotion);
  ClassDB::bind_method(D_METHOD("get_emotion"), &TaskShowPortrait::get_emotion);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "emotion"), "set_emotion",
               "get_emotion");
}

void TaskShowPortrait::on_start() {
  MysteryGameState *mgs = MysteryGameState::get_singleton();
  if (mgs) {
    mgs->request_portrait(character_id_, emotion_);
  }
  finished_ = true;
}

void TaskShowPortrait::complete_instantly() { on_start(); }

void TaskShowPortrait::set_character_id(const String &id) {
  character_id_ = id;
}
String TaskShowPortrait::get_character_id() const { return character_id_; }

void TaskShowPortrait::set_emotion(const String &emotion) {
  emotion_ = emotion;
}
String TaskShowPortrait::get_emotion() const { return emotion_; }

} // namespace mystery
