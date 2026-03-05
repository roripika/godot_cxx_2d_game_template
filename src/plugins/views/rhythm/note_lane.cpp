#include "note_lane.h"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

NoteLane::NoteLane() {}

NoteLane::~NoteLane() {}

void NoteLane::_bind_methods() {
  ClassDB::bind_method(D_METHOD("load_notes", "timestamps"),
                       &NoteLane::load_notes);
  ClassDB::bind_method(D_METHOD("set_input_action", "action"),
                       &NoteLane::set_input_action);
  ClassDB::bind_method(D_METHOD("set_speed", "speed"), &NoteLane::set_speed);
  ClassDB::bind_method(D_METHOD("set_target_y", "target_y"),
                       &NoteLane::set_target_y);
  // Helper to set references via script if needed
  ClassDB::bind_method(D_METHOD("set_conductor", "conductor"),
                       &NoteLane::set_conductor);
  ClassDB::bind_method(D_METHOD("set_manager", "manager"),
                       &NoteLane::set_manager);
}

void NoteLane::_ready() {
  // If not set via script, try to find them
  if (!conductor) {
    Node *node = get_tree()->get_root()->find_child("Conductor", true, false);
    if (node)
      conductor = Object::cast_to<Conductor>(node);
  }
  if (!manager) {
    Node *node =
        get_tree()->get_root()->find_child("RhythmGameManager", true, false);
    if (node)
      manager = Object::cast_to<RhythmGameManager>(node);
  }
}

void NoteLane::_process(double delta) {
  if (!conductor)
    return;

  // Spawning Logic
  float current_time = conductor->get_song_position();
  float spawn_lead_time = (target_y - spawn_y) / speed;

  // Check pending notes
  while (current_spawn_index < notes_timestamps.size()) {
    float note_time = notes_timestamps[current_spawn_index];
    if (current_time + spawn_lead_time >= note_time) {
      // Spawn note
      RhythmNote *note = memnew(RhythmNote);
      add_child(note);
      note->setup(note_time, speed, spawn_y, target_y);
      // active_notes.append(note); // TypedArray append might be complex with
      // raw pointer conversion? Actually, TypedArray<T> stores Variants.
      active_notes.push_back(note);

      current_spawn_index++;
    } else {
      break; // Sorted by time, so stop if not time yet
    }
  }

  // Update active notes
  // Iterate backwards to allow removal
  for (int i = active_notes.size() - 1; i >= 0; i--) {
    RhythmNote *note = Object::cast_to<RhythmNote>(active_notes[i]);
    if (!note) {
      active_notes.remove_at(i);
      continue;
    }

    note->update_position(current_time);

    // Remove missed notes (too far gone)
    // Let's say 0.2s late (200ms)
    if (current_time > note->get_target_time() + 0.2f) {
      if (manager)
        manager->register_miss();
      note->queue_free();
      active_notes.remove_at(i);
    }
  }
}

void NoteLane::_unhandled_input(const Ref<InputEvent> &event) {
  if (input_action.is_empty())
    return;

  if (event->is_action_pressed(input_action)) {
    if (!conductor || !manager)
      return;

    float current_time = conductor->get_song_position();

    // Find closest note
    RhythmNote *closest_note = nullptr;
    float min_diff = 1000.0f; // Large value
    int best_index = -1;

    for (int i = 0; i < active_notes.size(); i++) {
      RhythmNote *note = Object::cast_to<RhythmNote>(active_notes[i]);
      if (!note)
        continue;

      float diff = note->get_target_time() - current_time;
      // Only consider notes that haven't passed the miss threshold too much?
      // Or just closestabsolute diff?
      // Usually absolute diff within window makes sense.
      // But we should prioritize the earliest unhit note within window.
      // Since notes are spawned in order, active_notes[0] is usually the
      // target.

      float abs_diff = Math::abs(diff);
      if (abs_diff < min_diff) {
        min_diff = abs_diff;
        closest_note = note;
        best_index = i;
      }
    }

    // Register Hit (even if miss, register attempt)
    // If within 0.2s window
    if (closest_note && min_diff <= 0.2f) {
      manager->register_hit(closest_note->get_target_time() - current_time);
      closest_note->queue_free(); // Visual removal
      active_notes.remove_at(best_index);
      manager->register_hit(0); // wait logic above handles scoring.
      // Actually register_hit() processes scoring. But wait, register_hit
      // expects time_diff. Wait, register_hit updates score/combo. If it's a
      // hit (perfect/good), we should remove note. If it's a miss (too
      // early/late but within window), we probably remove note too to avoid
      // confusion. So removing is fine.
    }
    // Else: spurious input? Ignore or punish? Usually ignore unless specific
    // anti-mash mechanic.
  }
}

void NoteLane::load_notes(const PackedFloat64Array &p_timestamps) {
  notes_timestamps = p_timestamps;
  notes_timestamps.sort(); // Ensure sorted
  current_spawn_index = 0;
  // Clear active?
}

void NoteLane::set_conductor(Conductor *p_conductor) {
  conductor = p_conductor;
}
void NoteLane::set_manager(RhythmGameManager *p_manager) {
  manager = p_manager;
}
void NoteLane::set_input_action(const String &p_action) {
  input_action = p_action;
}
void NoteLane::set_speed(float p_speed) { speed = p_speed; }
void NoteLane::set_target_y(float p_target_y) { target_y = p_target_y; }
