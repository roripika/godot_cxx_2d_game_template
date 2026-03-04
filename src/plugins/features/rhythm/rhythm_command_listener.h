#ifndef RHYTHM_COMMAND_LISTENER_H
#define RHYTHM_COMMAND_LISTENER_H

#include "core/rhythm/conductor.h"
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class RhythmCommandListener : public Node {
  GDCLASS(RhythmCommandListener, Node)

private:
  Conductor *conductor = nullptr;

  // Timing window in seconds (e.g., 0.1s tolerance)
  float tolerance = 0.15f;

  // Current input sequence
  Array current_sequence;

  // Last input beat to prevent double inputs on same beat
  int last_input_beat = -1;

protected:
  static void _bind_methods();

public:
  RhythmCommandListener();
  ~RhythmCommandListener();

  void _ready() override;
  void _unhandled_input(const Ref<InputEvent> &event) override;

  void set_conductor(Conductor *p_conductor);

  // Command structure: Array of Strings ["pon", "pon", "pata", "pon"]
  // Returns true if command executed
  bool check_command(const Array &p_command);
};

#endif // RHYTHM_COMMAND_LISTENER_H
