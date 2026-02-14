#ifndef CONDUCTOR_H
#define CONDUCTOR_H

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class Conductor : public Node {
  GDCLASS(Conductor, Node)

private:
  AudioStreamPlayer *audio_player = nullptr;

  float bpm = 120.0f;
  float crotchet = 0.0f;      // Seconds per beat
  float offset = 0.0f;        // Seconds to delay
  float song_position = 0.0f; // Current song position in seconds
  float last_reported_beat = 0.0f;
  int beats_before_start = 0;
  int measure = 1;
  int beat = 1;

  bool is_playing = false;

protected:
  static void _bind_methods();

public:
  Conductor();
  ~Conductor();

  void _ready() override;
  void _process(double delta) override;

  void play(Ref<AudioStream> p_stream, float p_bpm, float p_offset = 0.0f,
            int p_beats_before_start = 0);
  void stop();

  // Getters & Setters
  void set_bpm(float p_bpm);
  float get_bpm() const;

  float get_song_position() const;
  float get_sec_per_beat() const;
  float get_beat_position() const; // Exact beat position (e.g. 4.5)
  int get_closest_beat(int p_quantize = 1) const; // Snap to grid

  // Latency compensation logic can be added later
  void report_beat();
};

#endif // CONDUCTOR_H
