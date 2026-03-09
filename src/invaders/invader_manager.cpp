#include "invader_manager.h"

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace invaders {

void InvaderManager::_bind_methods() {
  // --- シグナル ---
  ADD_SIGNAL(MethodInfo("score_changed",
                        PropertyInfo(Variant::INT, "new_score")));
  ADD_SIGNAL(MethodInfo("game_over"));
  ADD_SIGNAL(MethodInfo("game_clear"));

  // --- ゲームロジック API ---
  ClassDB::bind_method(D_METHOD("add_score", "points"),
                       &InvaderManager::add_score);
  ClassDB::bind_method(D_METHOD("notify_player_hit"),
                       &InvaderManager::notify_player_hit);
  ClassDB::bind_method(D_METHOD("reset"),
                       &InvaderManager::reset);

  // --- プロパティ ---
  ClassDB::bind_method(D_METHOD("get_current_score"),
                       &InvaderManager::get_current_score);
  ClassDB::bind_method(D_METHOD("set_current_score", "v"),
                       &InvaderManager::set_current_score);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "current_score"),
               "set_current_score", "get_current_score");

  ClassDB::bind_method(D_METHOD("get_game_over_y"),
                       &InvaderManager::get_game_over_y);
  ClassDB::bind_method(D_METHOD("set_game_over_y", "v"),
                       &InvaderManager::set_game_over_y);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "game_over_y"),
               "set_game_over_y", "get_game_over_y");

  ClassDB::bind_method(D_METHOD("get_enemy_group"),
                       &InvaderManager::get_enemy_group);
  ClassDB::bind_method(D_METHOD("set_enemy_group", "v"),
                       &InvaderManager::set_enemy_group);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "enemy_group"),
               "set_enemy_group", "get_enemy_group");
}

InvaderManager::InvaderManager() {}
InvaderManager::~InvaderManager() {}

// ------------------------------------------------------------------
// 毎フレーム監視
// ------------------------------------------------------------------

void InvaderManager::_process(double /*delta*/) {
  if (game_ended_) return;

  SceneTree *tree = get_tree();
  if (tree == nullptr) return;

  Array enemies = tree->get_nodes_in_group(enemy_group_);

  // 全滅チェック
  if (enemies.size() == 0) {
    game_ended_ = true;
    UtilityFunctions::print("[InvaderManager] game_clear! score=", current_score_);
    emit_signal("game_clear");
    return;
  }

  // 敵が下限 Y を超えていないかチェック
  for (int i = 0; i < enemies.size(); ++i) {
    Node2D *enemy = Object::cast_to<Node2D>(enemies[i]);
    if (enemy != nullptr && enemy->get_position().y >= game_over_y_) {
      emit_game_over();
      return;
    }
  }
}

// ------------------------------------------------------------------
// ゲームロジック API
// ------------------------------------------------------------------

void InvaderManager::add_score(int points) {
  if (game_ended_) return;
  current_score_ += points;
  UtilityFunctions::print("[InvaderManager] score=", current_score_);
  emit_signal("score_changed", current_score_);
}

void InvaderManager::notify_player_hit() {
  emit_game_over();
}

void InvaderManager::reset() {
  current_score_ = 0;
  game_ended_    = false;
  emit_signal("score_changed", current_score_);
}

// ------------------------------------------------------------------
// 内部ヘルパー
// ------------------------------------------------------------------

void InvaderManager::emit_game_over() {
  if (game_ended_) return;
  game_ended_ = true;
  UtilityFunctions::print("[InvaderManager] game_over! score=", current_score_);
  emit_signal("game_over");
}

// ------------------------------------------------------------------
// プロパティアクセサ
// ------------------------------------------------------------------

int   InvaderManager::get_current_score() const   { return current_score_; }
void  InvaderManager::set_current_score(int v)    { current_score_ = v;    }

float InvaderManager::get_game_over_y() const     { return game_over_y_;   }
void  InvaderManager::set_game_over_y(float v)    { game_over_y_    = v;   }

String InvaderManager::get_enemy_group() const    { return enemy_group_;   }
void   InvaderManager::set_enemy_group(const String &v) { enemy_group_ = v; }

} // namespace invaders
