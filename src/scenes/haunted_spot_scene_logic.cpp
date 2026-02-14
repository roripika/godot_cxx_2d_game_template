#include "haunted_spot_scene_logic.h"
#include "core/adventure_game_state.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/scene_tree_timer.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

HauntedSpotSceneLogic::HauntedSpotSceneLogic() {
  dialogue_ui = nullptr;
  interaction_manager = nullptr;
  ghost_sprite = nullptr;
}

HauntedSpotSceneLogic::~HauntedSpotSceneLogic() {}

void HauntedSpotSceneLogic::_bind_methods() {
  ClassDB::bind_method(D_METHOD("_on_clicked_at", "pos"),
                       &HauntedSpotSceneLogic::_on_clicked_at);
  ClassDB::bind_method(D_METHOD("_change_scene_callback"),
                       &HauntedSpotSceneLogic::_change_scene_callback);
}

void HauntedSpotSceneLogic::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  interaction_manager = Object::cast_to<InteractionManager>(
      get_node_or_null("InteractionManager"));
  if (interaction_manager) {
    interaction_manager->connect("clicked_at",
                                 Callable(this, "_on_clicked_at"));
  }

  Node *canvas_layer = get_node_or_null("CanvasLayer");
  if (canvas_layer) {
    dialogue_ui = Object::cast_to<DialogueUI>(
        canvas_layer->get_node_or_null("DialogueUI"));
  }

  Node *ghost_area = get_node_or_null("GhostArea");
  if (ghost_area) {
    ghost_sprite =
        Object::cast_to<Control>(ghost_area->get_node_or_null("Sprite2D"));
    if (ghost_sprite) {
      ghost_base_pos = ghost_sprite->get_position();
    }
  }

  if (dialogue_ui) {
    dialogue_ui->show_message("System",
                              "You arrived at the Haunted Spot.\nFind the "
                              "source of the paranormal activity.");
  }
}

void HauntedSpotSceneLogic::_process(double delta) {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  if (ghost_sprite) {
    double time = Time::get_singleton()->get_ticks_msec() / 200.0;
    float offset_y = Math::sin(time) * 10.0;
    ghost_sprite->set_position(
        Vector2(ghost_base_pos.x, ghost_base_pos.y + offset_y));
  }
}

void HauntedSpotSceneLogic::_on_clicked_at(Vector2 pos) {
  // Ghost Area (Approximation based on GDScript logic)
  // GDScript: $GhostArea.position is (576, 324)
  // Rect: pos - (32, 32), size (64, 64) -> Rect2(544, 292, 64, 64)
  Rect2 ghost_rect(544, 292, 64, 64);

  // Exit Area
  Rect2 exit_rect(0, 0, 100, 648);

  AdventureGameStateBase *state = AdventureGameStateBase::get_singleton();
  if (!state || !dialogue_ui)
    return;

  if (ghost_rect.has_point(pos)) {
    if (state->get_flag("has_evidence")) {
      dialogue_ui->show_message(
          "Detective",
          "I already have the evidence. I should report to the Boss.");
    } else {
      dialogue_ui->show_message(
          "Detective", "Aha! This ectoplasm proves it.\n(Evidence Collected)");
      state->set_flag("has_evidence", true);
      state->add_item("Ectoplasm Sample");
    }
  } else if (exit_rect.has_point(pos)) {
    dialogue_ui->show_message("System", "Returning to Office...");

    Ref<SceneTreeTimer> timer = get_tree()->create_timer(1.0);
    timer->connect("timeout", Callable(this, "_change_scene_callback"));
  } else {
    dialogue_ui->hide_dialogue();
  }
}

void HauntedSpotSceneLogic::_change_scene_callback() {
  AdventureGameStateBase *state = AdventureGameStateBase::get_singleton();
  if (state) {
    state->change_scene("res://samples/mystery/office_scene.tscn");
  }
}
