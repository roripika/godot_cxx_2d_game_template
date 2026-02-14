#ifndef FIGHTER_CONTROLLER_H
#define FIGHTER_CONTROLLER_H

#include "components/hitbox_component.h"
#include "components/hurtbox_component.h"
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/sprite2d.hpp>

// Assuming GameEntity is for RPG/Isometric context.
// For pure side-scrolling Fighter, we might inherit CharacterBody2D directly or
// use GameEntity if it supports side-scrolling well. Let's inherit
// CharacterBody2D for simpler Physics separation for now, or check GameEntity
// later. Actually GameEntity handles basic Movement, but Fighting games need
// specific frame data. Let's keep it simple: FighterController inherits
// CharacterBody2D.

using namespace godot;

class FighterController : public CharacterBody2D {
  GDCLASS(FighterController, CharacterBody2D)

public:
  enum State {
    IDLE,
    WALK_FORWARD,
    WALK_BACKWARD,
    CROUCH,
    JUMP,
    ATTACK_LIGHT,
    ATTACK_HEAVY,
    HIT_STUN,
    BLOCK,
    KO
  };

private:
  State current_state = IDLE;
  int player_id = 1; // 1 or 2
  double health = 100.0;
  double max_health; // Kept as it was not in the snippet's replacement

  // Movement Parameters
  double speed = 300.0;
  double jump_force = -600.0;
  double gravity = 1200.0;

  // Components
  godot::Sprite2D *sprite = nullptr;
  godot::AnimationPlayer *animation_player =
      nullptr;                         // Renamed from anim_player
  HitboxComponent *hitbox = nullptr;   // Refactored
  HurtboxComponent *hurtbox = nullptr; // Refactored

  void update_animation();
  void _on_taken_hit(HitboxComponent *hitbox, double damage, double hit_stun,
                     Vector2 knockback);
  void _on_hit_connected(HurtboxComponent *hurtbox);

protected:
  static void _bind_methods();

public:
  FighterController();
  ~FighterController();

  void _ready() override;
  void _physics_process(double delta) override;

  // Setters/Getters
  void set_player_id(int id);
  int get_player_id() const;

  // Actions
  void take_damage(double amount);
  double get_health() const;
  void change_state(State new_state);
};

#endif // FIGHTER_CONTROLLER_H
