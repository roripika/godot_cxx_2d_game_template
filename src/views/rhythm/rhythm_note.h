#ifndef RHYTHM_NOTE_H
#define RHYTHM_NOTE_H

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/sprite2d.hpp>

using namespace godot;

class RhythmNote : public Node2D {
  GDCLASS(RhythmNote, Node2D)

private:
  float target_time;
  float speed;
  float spawn_y;
  float target_y;

  Sprite2D *sprite = nullptr;

protected:
  static void _bind_methods();

public:
  RhythmNote();
  ~RhythmNote();

  void _ready() override;

  void setup(float p_target_time, float p_speed, float p_spawn_y,
             float p_target_y);
  void update_position(float current_song_pos);

  float get_target_time() const;

  // To allow setting a visual
  void set_texture(Ref<Texture2D> p_texture);
};

#endif // RHYTHM_NOTE_H
