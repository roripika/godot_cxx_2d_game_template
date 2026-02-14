#include "conductor.h"
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

Conductor::Conductor() {}

Conductor::~Conductor() {}

void Conductor::_bind_methods() {
  ClassDB::bind_method(
      D_METHOD("play", "stream", "bpm", "offset", "beats_before_start"),
      &Conductor::play, DEFVAL(0.0f), DEFVAL(0));
  ClassDB::bind_method(D_METHOD("stop"), &Conductor::stop);
  ClassDB::bind_method(D_METHOD("set_bpm", "bpm"), &Conductor::set_bpm);
  ClassDB::bind_method(D_METHOD("get_bpm"), &Conductor::get_bpm);
  ClassDB::bind_method(D_METHOD("get_song_position"),
                       &Conductor::get_song_position);
  ClassDB::bind_method(D_METHOD("get_sec_per_beat"),
                       &Conductor::get_sec_per_beat);
  ClassDB::bind_method(D_METHOD("get_beat_position"),
                       &Conductor::get_beat_position);
  ClassDB::bind_method(D_METHOD("get_closest_beat", "quantize"),
                       &Conductor::get_closest_beat, DEFVAL(1));

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bpm"), "set_bpm", "get_bpm");

  ADD_SIGNAL(MethodInfo("beat", PropertyInfo(Variant::INT, "position")));
  ADD_SIGNAL(MethodInfo("measure", PropertyInfo(Variant::INT, "position")));
}

void Conductor::_ready() {
  audio_player = memnew(AudioStreamPlayer);
  add_child(audio_player);
}

void Conductor::_process(double delta) {
  if (!is_playing || !audio_player)
    return;

  // Direct playback position is usually updated in chunks, so for smoother
  // visuals we'd interpolate. For rhythm logic, using get_playback_position() +
  // AudioServer delay is standard. song_position =
  // audio_player->get_playback_position() +
  // AudioServer::get_singleton()->get_time_since_last_mix() -
  // AudioServer::get_singleton()->get_output_latency(); Simplified for now:
  song_position = audio_player->get_playback_position();
  song_position -= offset;

  float actual_song_pos = song_position;

  // Check for beats
  if (song_position > 0) {
    float beat_pos_raw = song_position / crotchet;
    int current_beat = static_cast<int>(beat_pos_raw);

    if (current_beat > last_reported_beat) {
      last_reported_beat = current_beat;
      beat = current_beat + 1; // 1-indexed for musicians usually
      emit_signal("beat", beat);

      // Assume 4/4 time for now
      if ((beat - 1) % 4 == 0) {
        measure = (beat - 1) / 4 + 1;
        emit_signal("measure", measure);
      }
    }
  }
}

void Conductor::play(Ref<AudioStream> p_stream, float p_bpm, float p_offset,
                     int p_beats_before_start) {
  if (!audio_player)
    return;

  audio_player->set_stream(p_stream);
  set_bpm(p_bpm);
  offset = p_offset;
  beats_before_start = p_beats_before_start;

  song_position = -(crotchet * beats_before_start);
  last_reported_beat = -beats_before_start - 1;

  audio_player->play();
  is_playing = true;
  UtilityFunctions::print("Conductor: Playing BPM=", bpm,
                          " Crotchet=", crotchet);
}

void Conductor::stop() {
  if (audio_player)
    audio_player->stop();
  is_playing = false;
  song_position = 0.0f;
  last_reported_beat = 0.0f;
}

void Conductor::set_bpm(float p_bpm) {
  bpm = p_bpm;
  crotchet = 60.0f / bpm;
}

float Conductor::get_bpm() const { return bpm; }
float Conductor::get_song_position() const { return song_position; }
float Conductor::get_sec_per_beat() const { return crotchet; }

float Conductor::get_beat_position() const {
  if (crotchet == 0)
    return 0;
  return song_position / crotchet;
}

int Conductor::get_closest_beat(int p_quantize) const {
  if (crotchet == 0)
    return 0;
  return static_cast<int>(round(song_position / crotchet));
}
