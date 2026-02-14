#include "rhythm_command_listener.h"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

RhythmCommandListener::RhythmCommandListener() {}

RhythmCommandListener::~RhythmCommandListener() {}

void RhythmCommandListener::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_conductor", "conductor"),
                       &RhythmCommandListener::set_conductor);
  ClassDB::bind_method(D_METHOD("check_command", "command"),
                       &RhythmCommandListener::check_command);

  ADD_SIGNAL(MethodInfo("rhythm_input", PropertyInfo(Variant::STRING, "type"),
                        PropertyInfo(Variant::STRING, "judgment")));
  ADD_SIGNAL(
      MethodInfo("command_executed", PropertyInfo(Variant::ARRAY, "command")));
  ADD_SIGNAL(MethodInfo("rhythm_failed"));
}

void RhythmCommandListener::_ready() {
  if (!conductor) {
    Node *node = get_tree()->get_root()->find_child("Conductor", true, false);
    if (node)
      conductor = Object::cast_to<Conductor>(node);
  }
}

void RhythmCommandListener::_unhandled_input(const Ref<InputEvent> &event) {
  if (!conductor)
    return;

  String input_type = "";
  if (event->is_action_pressed("ui_accept") ||
      event->is_action_pressed("rhythm_pon")) {
    input_type = "pon"; // Circle/A
  } else if (event->is_action_pressed("ui_cancel") ||
             event->is_action_pressed("rhythm_pata")) {
    input_type = "pata"; // Square/X
  } else if (event->is_action_pressed("rhythm_chaka")) {
    input_type = "chaka"; // Triangle/Y
  } else if (event->is_action_pressed("rhythm_don")) {
    input_type = "don"; // Cross/B
  }

  if (!input_type.is_empty()) {
    float song_pos = conductor->get_song_position();
    float sec_per_beat = conductor->get_sec_per_beat();
    if (sec_per_beat <= 0)
      return;

    // Check offset from nearest beat
    // Ideally we want to know WHICH beat we are closest to
    float beat_pos = song_pos / sec_per_beat;
    int closest_beat = static_cast<int>(Math::round(beat_pos));
    float diff = (beat_pos - closest_beat) * sec_per_beat; // diff in seconds

    if (Math::abs(diff) <= tolerance) {
      // Hit!
      if (closest_beat != last_input_beat) {
        last_input_beat = closest_beat;
        current_sequence.push_back(input_type);
        emit_signal("rhythm_input", input_type, "perfect");

        // Max length check (e.g. 4)
        if (current_sequence.size() > 4) {
          current_sequence.pop_front();
        }
      }
    } else {
      // Miss / Off-beat
      if (current_sequence.size() > 0) {
        emit_signal("rhythm_failed");
        current_sequence.clear();
      }
      emit_signal("rhythm_input", input_type, "miss");
    }
  }
}

bool RhythmCommandListener::check_command(const Array &p_command) {
  if (current_sequence.size() < p_command.size())
    return false;

  // Check if the END of current_sequence matches p_command
  int start_index = current_sequence.size() - p_command.size();
  for (int i = 0; i < p_command.size(); i++) {
    if (current_sequence[start_index + i] != p_command[i]) {
      return false;
    }
  }

  // Match found!
  emit_signal("command_executed", p_command);
  current_sequence.clear();
  return true;
}

void RhythmCommandListener::set_conductor(Conductor *p_conductor) {
  conductor = p_conductor;
}
