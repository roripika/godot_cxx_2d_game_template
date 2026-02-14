#include "rhythm_game_manager.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

RhythmGameManager::RhythmGameManager() {}

RhythmGameManager::~RhythmGameManager() {}

void RhythmGameManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("register_hit", "time_diff"),
                       &RhythmGameManager::register_hit);
  ClassDB::bind_method(D_METHOD("register_miss"),
                       &RhythmGameManager::register_miss);
  ClassDB::bind_method(D_METHOD("reset"), &RhythmGameManager::reset);
  ClassDB::bind_method(D_METHOD("get_score"), &RhythmGameManager::get_score);
  ClassDB::bind_method(D_METHOD("get_combo"), &RhythmGameManager::get_combo);
  ClassDB::bind_method(D_METHOD("get_health"), &RhythmGameManager::get_health);
  ClassDB::bind_method(D_METHOD("set_windows", "perfect", "good"),
                       &RhythmGameManager::set_windows);

  ADD_SIGNAL(MethodInfo("score_changed", PropertyInfo(Variant::INT, "score")));
  ADD_SIGNAL(MethodInfo("combo_changed", PropertyInfo(Variant::INT, "combo")));
  ADD_SIGNAL(
      MethodInfo("health_changed", PropertyInfo(Variant::FLOAT, "health")));
  ADD_SIGNAL(MethodInfo("judgment", PropertyInfo(Variant::STRING, "type")));
}

void RhythmGameManager::register_hit(float time_diff) {
  float abs_diff = Math::abs(time_diff);

  String judgment_type = "miss";
  int score_add = 0;

  if (abs_diff <= perfect_window) {
    judgment_type = "perfect";
    score_add = 100 + (combo * combo_bonus);
    combo++;
    health = Math::min(100.0f, health + 1.0f);
  } else if (abs_diff <= good_window) {
    judgment_type = "good";
    score_add = 50 + (combo * combo_bonus * 0.5f);
    combo++;
    health = Math::min(100.0f, health + 0.5f);
  } else {
    // Late/Early Miss
    judgment_type = "miss";
    combo = 0;
    health = Math::max(0.0f, health - miss_penalty);
  }

  score += score_add;

  emit_signal("score_changed", score);
  emit_signal("combo_changed", combo);
  emit_signal("health_changed", health);
  emit_signal("judgment", judgment_type);
}

void RhythmGameManager::register_miss() {
  combo = 0;
  health = Math::max(0.0f, health - miss_penalty);

  emit_signal("combo_changed", combo);
  emit_signal("health_changed", health);
  emit_signal("judgment", "miss");
}

void RhythmGameManager::reset() {
  score = 0;
  combo = 0;
  health = 100.0f;
  emit_signal("score_changed", score);
  emit_signal("combo_changed", combo);
  emit_signal("health_changed", health);
}

int RhythmGameManager::get_score() const { return score; }
int RhythmGameManager::get_combo() const { return combo; }
float RhythmGameManager::get_health() const { return health; }

void RhythmGameManager::set_windows(float p_perfect, float p_good) {
  perfect_window = p_perfect;
  good_window = p_good;
}
