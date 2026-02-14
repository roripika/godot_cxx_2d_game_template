#include "fighting_game_manager.h"
#include "../entities/fighter_controller.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

FightingGameManager::FightingGameManager() {
  current_state = WAITING;
  round_time = 99.0;
  p1_wins = 0;
  p2_wins = 0;
  timer_label = nullptr;
  message_label = nullptr;
  p1_health_bar = nullptr;
  p2_health_bar = nullptr;
  p1_combo_label = nullptr;
  p2_combo_label = nullptr;
}

FightingGameManager::~FightingGameManager() {}

void FightingGameManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("start_round"),
                       &FightingGameManager::start_round);
  ClassDB::bind_method(D_METHOD("on_hit_confirmed", "player_id"),
                       &FightingGameManager::on_hit_confirmed);
}

void FightingGameManager::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  // Resolve UI nodes (Expects a specific scene structure)
  Node *canvas = get_node_or_null("CanvasLayer");
  if (canvas) {
    Node *hud = canvas->get_node_or_null("HUD");
    if (hud) {
      timer_label = Object::cast_to<Label>(hud->get_node_or_null("TimerLabel"));
      message_label =
          Object::cast_to<Label>(hud->get_node_or_null("MessageLabel"));
      p1_health_bar = Object::cast_to<TextureProgressBar>(
          hud->get_node_or_null("P1HealthrBar")); // Typo kept as is in scene
      p2_health_bar = Object::cast_to<TextureProgressBar>(
          hud->get_node_or_null("P2HealthBar"));
      p1_combo_label =
          Object::cast_to<Label>(hud->get_node_or_null("P1ComboLabel"));
      p2_combo_label =
          Object::cast_to<Label>(hud->get_node_or_null("P2ComboLabel"));
    }
  }

  // Resolve Fighters
  // Resolve Fighters
  fighter1 = Object::cast_to<FighterController>(get_node_or_null("FighterP1"));
  if (fighter1) {
    fighter1->connect("hit_confirmed",
                      Callable(this, "on_hit_confirmed").bind(1));
  }
  fighter2 = Object::cast_to<FighterController>(get_node_or_null("FighterP2"));
  if (fighter2) {
    fighter2->connect("hit_confirmed",
                      Callable(this, "on_hit_confirmed").bind(2));
  }

  start_round();
}

void FightingGameManager::_process(double delta) {
  if (current_state == FIGHTING) {
    round_time -= delta;
    if (round_time <= 0) {
      round_time = 0;
      end_round(0); // Time Over -> Draw (simplified)
    }

    // Combo Timer Logic
    if (p1_combo > 0) {
      p1_combo_timer -= delta;
      if (p1_combo_timer <= 0) {
        p1_combo = 0;
        if (p1_combo_label)
          p1_combo_label->set_visible(false);
      }
    }
    if (p2_combo > 0) {
      p2_combo_timer -= delta;
      if (p2_combo_timer <= 0) {
        p2_combo = 0;
        if (p2_combo_label)
          p2_combo_label->set_visible(false);
      }
    }

    update_ui();
  }
}

void FightingGameManager::start_round() {
  current_state = COUNTDOWN;
  round_time = 99.0;
  if (message_label) {
    message_label->set_text("FIGHT!");
    // In a real game, we'd use a timer/tween for "3, 2, 1, FIGHT"
  }
  current_state = FIGHTING;
}

void FightingGameManager::end_round(int winner_id) {
  current_state = ROUND_OVER;
  if (message_label) {
    if (winner_id == 0)
      message_label->set_text("TIME OVER");
    else if (winner_id == 1)
      message_label->set_text("PLAYER 1 WINS");
    else
      message_label->set_text("PLAYER 2 WINS");
  }
}

void FightingGameManager::update_ui() {
  if (timer_label) {
    timer_label->set_text(String::num((int)round_time));
  }
}

void FightingGameManager::on_hit_confirmed(int player_id) {
  if (player_id == 1) {
    p1_combo++;
    p1_combo_timer = 2.0; // Reset timer
    if (p1_combo_label) {
      p1_combo_label->set_text(String::num(p1_combo) + " HITS!");
      p1_combo_label->set_visible(true);
    }
  } else if (player_id == 2) {
    p2_combo++;
    p2_combo_timer = 2.0; // Reset timer
    if (p2_combo_label) {
      p2_combo_label->set_text(String::num(p2_combo) + " HITS!");
      p2_combo_label->set_visible(true);
    }
  }
}
