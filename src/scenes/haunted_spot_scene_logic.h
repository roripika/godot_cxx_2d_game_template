#ifndef HAUNTED_SPOT_SCENE_LOGIC_H
#define HAUNTED_SPOT_SCENE_LOGIC_H

#include "core/dialogue_ui.h"
#include "core/interaction_manager.h"
#include <godot_cpp/classes/control.hpp> // For ColorRect ghost
#include <godot_cpp/classes/node2d.hpp>

using namespace godot;

class HauntedSpotSceneLogic : public Node2D {
  GDCLASS(HauntedSpotSceneLogic, Node2D)

private:
  DialogueUI *dialogue_ui;
  InteractionManager *interaction_manager;
  Control *ghost_sprite; // It is a ColorRect in the tscn
  Vector2 ghost_base_pos;

protected:
  static void _bind_methods();

public:
  HauntedSpotSceneLogic();
  ~HauntedSpotSceneLogic();

  void _ready() override;
  void _process(double delta) override;

  // Signal callbacks
  void _on_clicked_at(Vector2 pos);
  void _change_scene_callback();
};

#endif // HAUNTED_SPOT_SCENE_LOGIC_H
