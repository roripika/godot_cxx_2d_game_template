#ifndef RHYTHM_GAME_MANAGER_H
#define RHYTHM_GAME_MANAGER_H

#include <godot_cpp/classes/node.hpp>

using namespace godot;

class RhythmGameManager : public Node {
  GDCLASS(RhythmGameManager, Node)

private:
  int score = 0;
  int combo = 0;
  float health = 100.0f;

  // Judgment windows in seconds
  float perfect_window = 0.05f; // 50ms
  float good_window = 0.1f;     // 100ms
  float miss_penalty = 10.0f;
  float combo_bonus = 1.0f;

protected:
  static void _bind_methods();

public:
  RhythmGameManager();
  ~RhythmGameManager();

  void register_hit(float time_diff); // diff between input time and target time
  void register_miss();

  void reset();

  // Getters
  int get_score() const;
  int get_combo() const;
  float get_health() const;

  // Settings
  void set_windows(float p_perfect, float p_good);
};

#endif // RHYTHM_GAME_MANAGER_H
