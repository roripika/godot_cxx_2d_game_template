#ifndef FIGHTING_GAME_MANAGER_H
#define FIGHTING_GAME_MANAGER_H

#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/texture_progress_bar.hpp>

using namespace godot;

class FighterController; // Forward declaration

class FightingGameManager : public Node {
  GDCLASS(FightingGameManager, Node)

private:
  // Round State
  enum GameState { WAITING, COUNTDOWN, FIGHTING, ROUND_OVER };

  GameState current_state;
  double round_time;
  int p1_wins;
  int p2_wins;

  // Combo System
  int p1_combo = 0;
  int p2_combo = 0;
  double p1_combo_timer = 0.0;
  double p2_combo_timer = 0.0;

  // UI References
  Label *timer_label;
  Label *message_label;
  TextureProgressBar *p1_health_bar;
  TextureProgressBar *p2_health_bar;
  Label *p1_combo_label; // Added
  Label *p2_combo_label; // Added

  // Fighter References
  FighterController *fighter1 = nullptr;
  FighterController *fighter2 = nullptr;

protected:
  static void _bind_methods();

public:
  FightingGameManager();
  ~FightingGameManager();

  void _ready() override;
  void _process(double delta) override;

  void start_round();
  void end_round(int winner_id); // 0=Draw, 1=P1, 2=P2
  void update_ui();
  void on_hit_confirmed(int player_id); // Added
};

#endif // FIGHTING_GAME_MANAGER_H
