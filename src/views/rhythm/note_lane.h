#ifndef NOTE_LANE_H
#define NOTE_LANE_H

#include "core/rhythm/conductor.h"
#include "core/rhythm/rhythm_game_manager.h"
#include "rhythm_note.h"
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node2d.hpp>

using namespace godot;

class NoteLane : public Node2D {
  GDCLASS(NoteLane, Node2D)

private:
  Conductor *conductor = nullptr;
  RhythmGameManager *manager = nullptr;

  String input_action;
  float speed = 200.0f;
  float spawn_y = -50.0f;
  float target_y = 500.0f;

  PackedFloat64Array
      notes_timestamps; // Changed from NoteData for now to simple array
  int current_spawn_index = 0;

  TypedArray<RhythmNote> active_notes; // Stores RhythmNote*

protected:
  static void _bind_methods();

public:
  NoteLane();
  ~NoteLane();

  void _ready() override;
  void _process(double delta) override;
  void _unhandled_input(const Ref<InputEvent> &event) override;

  void load_notes(const PackedFloat64Array &p_timestamps);
  void set_conductor(Conductor *p_conductor);
  void set_manager(RhythmGameManager *p_manager);

  void set_input_action(const String &p_action);
  void set_speed(float p_speed);
  void set_target_y(float p_target_y);
};

#endif // NOTE_LANE_H
