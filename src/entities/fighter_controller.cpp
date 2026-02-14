#include "fighter_controller.h"
#include "core/hit_stop_manager.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>

using namespace godot;

FighterController::FighterController() {
  player_id = 1;
  health = 100.0;
  max_health = 100.0;
  speed = 300.0;
  jump_force = -600.0;
  gravity = 1800.0; // Strong gravity for snappy jumps
  current_state = IDLE;
  sprite = nullptr;
  animation_player = nullptr;
}

FighterController::~FighterController() {}

void FighterController::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_player_id", "id"),
                       &FighterController::set_player_id);
  ClassDB::bind_method(D_METHOD("get_player_id"),
                       &FighterController::get_player_id);
  ClassDB::bind_method(D_METHOD("take_damage", "amount"),
                       &FighterController::take_damage);
  ClassDB::bind_method(D_METHOD("get_health"), &FighterController::get_health);
  ClassDB::bind_method(
      D_METHOD("_on_taken_hit", "hitbox", "damage", "hit_stun", "knockback"),
      &FighterController::_on_taken_hit);
  ClassDB::bind_method(D_METHOD("_on_hit_connected", "hurtbox"),
                       &FighterController::_on_hit_connected);

  ADD_PROPERTY(PropertyInfo(Variant::INT, "player_id"), "set_player_id",
               "get_player_id");

  ADD_SIGNAL(MethodInfo("hit_confirmed"));
}

void FighterController::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  sprite = Object::cast_to<Sprite2D>(get_node_or_null("Sprite2D"));
  animation_player =
      Object::cast_to<AnimationPlayer>(get_node_or_null("AnimationPlayer"));

  hitbox = Object::cast_to<HitboxComponent>(get_node_or_null("Hitbox"));
  if (hitbox) {
    hitbox->set_monitoring(false); // Disabled by default
    hitbox->connect("hit_connected", Callable(this, "_on_hit_connected"));
  }

  hurtbox = Object::cast_to<HurtboxComponent>(get_node_or_null("Hurtbox"));
  if (hurtbox) {
    hurtbox->connect("taken_hit", Callable(this, "_on_taken_hit"));
  }
}

void FighterController::_on_hit_connected(HurtboxComponent *hurtbox) {
  // Hit logic
  emit_signal("hit_confirmed");

  if (hitbox) {
    hitbox->set_deferred("monitoring", false);
  }

  // Hit Stop for attacker (Hit Lag)
  HitStopManager *hit_stop = HitStopManager::get_singleton();
  if (hit_stop) {
    hit_stop->apply_hit_stop(0.1, 0.0);
  }
}

void FighterController::_on_taken_hit(HitboxComponent *hitbox, double damage,
                                      double hit_stun, Vector2 knockback) {
  take_damage(damage);

  // Apply Knockback (Simplistic)
  // set_velocity(knockback);
  // TODO: Add knockback state or impulse

  // Hit Stop for victim
  HitStopManager *hit_stop = HitStopManager::get_singleton();
  if (hit_stop) {
    hit_stop->apply_hit_stop(0.1, 0.0);
  }
}

void FighterController::update_animation() {
  // ... (existing logic or placeholder)
}

void FighterController::change_state(State new_state) {
  current_state = new_state;

  // Hitbox Management
  if (hitbox) {
    if (current_state == ATTACK_LIGHT || current_state == ATTACK_HEAVY) {
      hitbox->set_deferred("monitoring", true);
      // Auto-revert to IDLE after a short delay if no animation is controlling
      // it? For this demo, we rely on the AnimationPlayer or a simple timer.
      // Since we don't have animation events yet, let's just cheat and reset to
      // IDLE in process after a duration? Or rely on the physics process to
      // switch back if attack is done. Actually, let's keep it simple: Enable
      // here.
    } else {
      hitbox->set_deferred("monitoring", false);
    }
  }
}

void FighterController::_physics_process(double delta) {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  if (current_state == KO || current_state == HIT_STUN) {
    // Apply gravity but no input
    Vector2 velocity = get_velocity();
    velocity.y += gravity * delta;
    set_velocity(velocity);
    move_and_slide();
    return;
  }

  // Temporary: Reset attack state after small timer using delta for demo
  // purposes In real implementation, AnimationPlayer 'finished' signal would
  // drive this.
  static double attack_timer = 0.0;
  if (current_state == ATTACK_LIGHT) {
    attack_timer += delta;
    if (attack_timer > 0.2) { // 200ms attack
      attack_timer = 0;
      change_state(IDLE);
    }
    // Stop moving during attack
    set_velocity(Vector2(0, get_velocity().y + gravity * delta));
    move_and_slide();
    return;
  }

  Input *input = Input::get_singleton();
  Vector2 velocity = get_velocity();

  // Gravity
  if (!is_on_floor()) {
    velocity.y += gravity * delta;
  }

  // Input Handling (Simple Left/Right)
  // Assuming Input Map: "p1_move_left", "p1_move_right", "p1_jump", "p1_attack"
  String action_prefix = (player_id == 1) ? "p1_" : "p2_";

  double direction = input->get_axis(action_prefix + "move_left",
                                     action_prefix + "move_right");

  if (direction != 0) {
    velocity.x = direction * speed;
    if (direction > 0)
      change_state(WALK_FORWARD); // Assuming facing right
    else
      change_state(WALK_BACKWARD);

    // Flip sprite based on direction (basic)
    if (sprite) {
      sprite->set_flip_h(direction < 0);
    }
  } else {
    velocity.x = UtilityFunctions::move_toward(velocity.x, 0, speed);
    change_state(IDLE);
  }

  if (input->is_action_just_pressed(action_prefix + "jump") && is_on_floor()) {
    velocity.y = jump_force;
    change_state(JUMP);
  }

  if (input->is_action_just_pressed(action_prefix + "attack")) {
    change_state(ATTACK_LIGHT);
    // Attack logic (hitbox activation) handled in change_state
  }

  set_velocity(velocity);
  move_and_slide();
}

void FighterController::set_player_id(int id) { player_id = id; }

int FighterController::get_player_id() const { return player_id; }

void FighterController::take_damage(double amount) {
  health -= amount;
  if (health <= 0) {
    health = 0;
    change_state(KO);
  } else {
    change_state(HIT_STUN);
  }
  UtilityFunctions::print("Fighter ", player_id, " took ", amount,
                          " damage. HP: ", health);
}

double FighterController::get_health() const { return health; }
