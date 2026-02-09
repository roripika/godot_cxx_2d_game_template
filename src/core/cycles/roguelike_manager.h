#ifndef ROGUELIKE_MANAGER_H
#define ROGUELIKE_MANAGER_H

#include "../universal_world_data.h"
#include "roguelike_generator.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/tile_map_layer.hpp>

using namespace godot;

class RoguelikeManager : public Node {
  GDCLASS(RoguelikeManager, Node)

private:
  Ref<UniversalWorldData> world_data;
  Ref<RoguelikeGenerator> generator;

  // Properties to be set in Editor
  NodePath view_path;
  Ref<PackedScene> player_scene;

  int map_width;
  int map_height;
  int room_count;

protected:
  static void _bind_methods();

public:
  RoguelikeManager();
  ~RoguelikeManager();

  void _ready() override;

  // Generates a new level and spawns player
  void start_level();

  // Getters/Setters
  void set_view_path(const NodePath &p_path);
  NodePath get_view_path() const;

  void set_player_scene(const Ref<PackedScene> &p_scene);
  Ref<PackedScene> get_player_scene() const;

  void set_map_width(int p_width);
  int get_map_width() const;

  void set_map_height(int p_height);
  int get_map_height() const;

  void set_room_count(int p_count);
  int get_room_count() const;
};

#endif
