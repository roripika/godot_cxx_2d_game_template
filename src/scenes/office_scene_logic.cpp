#include "office_scene_logic.h"
#include "core/adventure_game_state.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/scene_tree_timer.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OfficeSceneLogic::OfficeSceneLogic() {
  dialogue_ui = nullptr;
  interaction_manager = nullptr;
}

OfficeSceneLogic::~OfficeSceneLogic() {}

void OfficeSceneLogic::_bind_methods() {
  ClassDB::bind_method(D_METHOD("_on_clicked_at", "pos"),
                       &OfficeSceneLogic::_on_clicked_at);
  ClassDB::bind_method(D_METHOD("_change_scene_callback"),
                       &OfficeSceneLogic::_change_scene_callback);
}

void OfficeSceneLogic::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  interaction_manager = Object::cast_to<InteractionManager>(
      get_node_or_null("InteractionManager"));
  if (interaction_manager) {
    interaction_manager->connect("clicked_at",
                                 Callable(this, "_on_clicked_at"));
  } else {
    UtilityFunctions::print("OfficeSceneLogic: InteractionManager not found!");
  }

  Node *canvas_layer = get_node_or_null("CanvasLayer");
  if (canvas_layer) {
    dialogue_ui = Object::cast_to<DialogueUI>(
        canvas_layer->get_node_or_null("DialogueUI"));
  }

  if (dialogue_ui) {
    AdventureGameStateBase *state = AdventureGameStateBase::get_singleton();
    if (state) {
      if (!state->get_flag("intro_done")) {
        dialogue_ui->show_message(
            "Boss", "Detectives! We have a new case.\nA Level 3 Apparition at "
                    "the old warehouse.\nGo investigate properly.");
        state->set_flag("intro_done", true);
      } else {
        dialogue_ui->show_message("Boss", "Back already? Did you solve it?");
      }
    }
  }
}

void OfficeSceneLogic::_on_clicked_at(Vector2 pos) {
  // Door Area
  Rect2 door_rect(900, 200, 100, 300);
  // Boss Area
  Rect2 boss_rect(100, 200, 150, 300);

  AdventureGameStateBase *state = AdventureGameStateBase::get_singleton();
  if (!state || !dialogue_ui)
    return;

  if (door_rect.has_point(pos)) {
    if (state->get_flag("case_solved")) {
      dialogue_ui->show_message("System", "Case Closed. Thanks for playing!");
    } else {
      dialogue_ui->show_message("System", "Going to the Haunted Spot...");

      // Create timer for delay
      Ref<SceneTreeTimer> timer = get_tree()->create_timer(1.0);
      timer->connect("timeout", Callable(this, "_change_scene_callback"));
    }
  } else if (boss_rect.has_point(pos)) {
    if (state->get_flag("has_evidence")) {
      dialogue_ui->show_message("Boss",
                                "Great work! That evidence proves it was a "
                                "poltergeist.\nYou solved the case!");
      state->set_flag("case_solved", true);
    } else {
      dialogue_ui->show_message(
          "Boss", "What are you doing here? Go to the warehouse!");
    }
  }
}

void OfficeSceneLogic::_change_scene_callback() {
  AdventureGameStateBase *state = AdventureGameStateBase::get_singleton();
  if (state) {
    state->change_scene("res://samples/mystery/demo_adv.tscn");
  }
}
