#include "rhythm_note.h"
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

RhythmNote::RhythmNote() {
  target_time = 0.0f;
  speed = 100.0f; // Pixels per second maybe?
  spawn_y = -50.0f;
  target_y = 500.0f;
}

RhythmNote::~RhythmNote() {}

void RhythmNote::_bind_methods() {
  ClassDB::bind_method(
      D_METHOD("setup", "target_time", "speed", "spawn_y", "target_y"),
      &RhythmNote::setup);
  ClassDB::bind_method(D_METHOD("update_position", "current_song_pos"),
                       &RhythmNote::update_position);
  ClassDB::bind_method(D_METHOD("get_target_time"),
                       &RhythmNote::get_target_time);
  ClassDB::bind_method(D_METHOD("set_texture", "texture"),
                       &RhythmNote::set_texture);
}

void RhythmNote::_ready() {
  sprite = memnew(Sprite2D);
  add_child(sprite);
}

void RhythmNote::setup(float p_target_time, float p_speed, float p_spawn_y,
                       float p_target_y) {
  target_time = p_target_time;
  speed = p_speed;
  spawn_y = p_spawn_y;
  target_y = p_target_y;
}

void RhythmNote::update_position(float current_song_pos) {
  // Time remaining until hit
  float time_diff = target_time - current_song_pos;

  // Position = TargetY - (TimeDiff * Speed)
  // If Speed is pixels/sec, then if TimeDiff is 1s, it's TargetY - Speed.
  // Wait, usually notes fall DOWN.
  // If TimeDiff is positive (future note), Y should be < TargetY.
  // So Y = TargetY - (TimeDiff * Speed).

  float new_y = target_y - (time_diff * speed);
  set_position(Vector2(0, new_y));

  // Visibility optimization?
  // setVisible(new_y >= spawn_y && new_y <= target_y + 100);
}

float RhythmNote::get_target_time() const { return target_time; }

void RhythmNote::set_texture(Ref<Texture2D> p_texture) {
  if (sprite) {
    sprite->set_texture(p_texture);
  }
}
