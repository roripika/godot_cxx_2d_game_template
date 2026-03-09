#include "invader_manager.h"
#include "invader_enemy.h"

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace invaders {

void InvaderManager::_bind_methods() {
  // --- シグナル ---
  ADD_SIGNAL(
      MethodInfo("score_changed", PropertyInfo(Variant::INT, "new_score")));
  ADD_SIGNAL(MethodInfo("game_over"));
  ADD_SIGNAL(MethodInfo("game_clear"));

  // --- ゲームロジック API ---
  ClassDB::bind_method(D_METHOD("add_score", "points"),
                       &InvaderManager::add_score);
  ClassDB::bind_method(D_METHOD("notify_player_hit"),
                       &InvaderManager::notify_player_hit);
  ClassDB::bind_method(D_METHOD("reset"), &InvaderManager::reset);

  // --- プロパティ ---
  ClassDB::bind_method(D_METHOD("get_current_score"),
                       &InvaderManager::get_current_score);
  ClassDB::bind_method(D_METHOD("set_current_score", "v"),
                       &InvaderManager::set_current_score);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "current_score"), "set_current_score",
               "get_current_score");

  ClassDB::bind_method(D_METHOD("get_game_over_y"),
                       &InvaderManager::get_game_over_y);
  ClassDB::bind_method(D_METHOD("set_game_over_y", "v"),
                       &InvaderManager::set_game_over_y);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "game_over_y"), "set_game_over_y",
               "get_game_over_y");

  ClassDB::bind_method(D_METHOD("get_enemy_group"),
                       &InvaderManager::get_enemy_group);
  ClassDB::bind_method(D_METHOD("set_enemy_group", "v"),
                       &InvaderManager::set_enemy_group);
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "enemy_group"), "set_enemy_group",
               "get_enemy_group");

  ClassDB::bind_method(D_METHOD("get_base_move_speed"),
                       &InvaderManager::get_base_move_speed);
  ClassDB::bind_method(D_METHOD("set_base_move_speed", "v"),
                       &InvaderManager::set_base_move_speed);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "base_move_speed"),
               "set_base_move_speed", "get_base_move_speed");

  ClassDB::bind_method(D_METHOD("get_max_speed_multiplier"),
                       &InvaderManager::get_max_speed_multiplier);
  ClassDB::bind_method(D_METHOD("set_max_speed_multiplier", "v"),
                       &InvaderManager::set_max_speed_multiplier);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_speed_multiplier"),
               "set_max_speed_multiplier", "get_max_speed_multiplier");
}

InvaderManager::InvaderManager() {}
InvaderManager::~InvaderManager() {}

// ------------------------------------------------------------------
// 毎フレーム監視
// ------------------------------------------------------------------

void InvaderManager::_process(double /*delta*/) {
  if (game_ended_)
    return;

  SceneTree *tree = get_tree();
  if (tree == nullptr)
    return;

  Array enemies = tree->get_nodes_in_group(enemy_group_);

  // 初期化待ち: 敵が1つ以上登録されるまで待つ
  if (!initialized_) {
    if (enemies.size() > 0) {
      initialized_ = true;
      initial_enemy_count_ = enemies.size();
      UtilityFunctions::print("[InvaderManager] Initialized with ",
                              initial_enemy_count_, " enemies.");
    }
    return;
  }

  // 速度倍率の計算 (1.0 〜 max_speed_multiplier_)
  float remaining_ratio =
      1.0f - (static_cast<float>(enemies.size()) / initial_enemy_count_);
  float speed_multiplier =
      1.0f + (max_speed_multiplier_ - 1.0f) * remaining_ratio;
  float current_speed = base_move_speed_ * speed_multiplier;

  // 全滅チェック (初期化後のみ)
  if (enemies.size() == 0) {
    game_ended_ = true;
    UtilityFunctions::print("[InvaderManager] game_clear! score=",
                            current_score_);
    emit_signal("game_clear");
    return;
  }

  // 敵の更新（速度アップ含む監視）
  for (int i = 0; i < enemies.size(); ++i) {
    InvaderEnemy *enemy = Object::cast_to<InvaderEnemy>(enemies[i]);
    if (enemy == nullptr)
      continue;

    // 速度を動的に更新
    enemy->set_move_speed(current_speed);

    // 下限 Y チェック
    if (enemy->get_position().y >= game_over_y_) {
      emit_game_over();
      return;
    }
  }
}

// ------------------------------------------------------------------
// ゲームロジック API
// ------------------------------------------------------------------

void InvaderManager::add_score(int points) {
  if (game_ended_)
    return;
  current_score_ += points;
  UtilityFunctions::print("[InvaderManager] score=", current_score_);
  emit_signal("score_changed", current_score_);
}

void InvaderManager::notify_player_hit() { emit_game_over(); }

void InvaderManager::reset() {
  current_score_ = 0;
  game_ended_ = false;
  emit_signal("score_changed", current_score_);
}

// ------------------------------------------------------------------
// 内部ヘルパー
// ------------------------------------------------------------------

void InvaderManager::emit_game_over() {
  if (game_ended_)
    return;
  game_ended_ = true;
  UtilityFunctions::print("[InvaderManager] game_over! score=", current_score_);
  emit_signal("game_over");
}

// ------------------------------------------------------------------
// プロパティアクセサ
// ------------------------------------------------------------------

int InvaderManager::get_current_score() const { return current_score_; }
void InvaderManager::set_current_score(int v) { current_score_ = v; }

float InvaderManager::get_game_over_y() const { return game_over_y_; }
void InvaderManager::set_game_over_y(float v) { game_over_y_ = v; }

String InvaderManager::get_enemy_group() const { return enemy_group_; }
void InvaderManager::set_enemy_group(const String &v) { enemy_group_ = v; }

float InvaderManager::get_base_move_speed() const { return base_move_speed_; }
void InvaderManager::set_base_move_speed(float v) { base_move_speed_ = v; }

float InvaderManager::get_max_speed_multiplier() const {
  return max_speed_multiplier_;
}
void InvaderManager::set_max_speed_multiplier(float v) {
  max_speed_multiplier_ = v;
}

} // namespace invaders
