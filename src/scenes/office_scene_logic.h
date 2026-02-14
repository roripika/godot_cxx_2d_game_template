#ifndef OFFICE_SCENE_LOGIC_H
#define OFFICE_SCENE_LOGIC_H

#include "core/dialogue_ui.h"
#include "core/interaction_manager.h"
#include <godot_cpp/classes/canvas_layer.hpp> // For getting DialogueUI
#include <godot_cpp/classes/node2d.hpp>

using namespace godot;

class OfficeSceneLogic : public Node2D {
  GDCLASS(OfficeSceneLogic, Node2D)

private:
  DialogueUI *dialogue_ui;
  InteractionManager *interaction_manager;

protected:
  static void _bind_methods();

public:
  OfficeSceneLogic();
  ~OfficeSceneLogic();

  void _ready() override;

  // Signal callback
  void _on_clicked_at(Vector2 pos);
  void _change_scene_callback();
};

#endif // OFFICE_SCENE_LOGIC_H
