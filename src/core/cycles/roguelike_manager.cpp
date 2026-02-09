#include "roguelike_manager.h"
#include "../../views/isometric_view.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void RoguelikeManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("start_level"), &RoguelikeManager::start_level);

  ClassDB::bind_method(D_METHOD("set_view_path", "path"),
                       &RoguelikeManager::set_view_path);
  ClassDB::bind_method(D_METHOD("get_view_path"),
                       &RoguelikeManager::get_view_path);

  ClassDB::bind_method(D_METHOD("set_player_scene", "scene"),
                       &RoguelikeManager::set_player_scene);
  ClassDB::bind_method(D_METHOD("get_player_scene"),
                       &RoguelikeManager::get_player_scene);

  ClassDB::bind_method(D_METHOD("set_map_width", "width"),
                       &RoguelikeManager::set_map_width);
  ClassDB::bind_method(D_METHOD("get_map_width"),
                       &RoguelikeManager::get_map_width);

  ClassDB::bind_method(D_METHOD("set_map_height", "height"),
                       &RoguelikeManager::set_map_height);
  ClassDB::bind_method(D_METHOD("get_map_height"),
                       &RoguelikeManager::get_map_height);

  ClassDB::bind_method(D_METHOD("set_room_count", "count"),
                       &RoguelikeManager::set_room_count);
  ClassDB::bind_method(D_METHOD("get_room_count"),
                       &RoguelikeManager::get_room_count);

  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "view_path"), "set_view_path",
               "get_view_path");
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_scene",
                            PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
               "set_player_scene", "get_player_scene");
  ADD_GROUP("Generation Settings", "map_");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "map_width"), "set_map_width",
               "get_map_width");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "map_height"), "set_map_height",
               "get_map_height");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "room_count"), "set_room_count",
               "get_room_count");
}

RoguelikeManager::RoguelikeManager() {
  map_width = 50;
  map_height = 50;
  room_count = 10;
}

RoguelikeManager::~RoguelikeManager() {}

void RoguelikeManager::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  start_level();
}

void RoguelikeManager::start_level() {
  world_data.instantiate();
  world_data->initialize(
      Vector3i(map_width, 2, map_height)); // Y=2 layers (0:floor, 1:wall?)

  generator.instantiate();
  generator->generate_dungeon(world_data, map_width, map_height, 0, room_count);

  // Assign to View
  if (!view_path.is_empty()) {
    Node *node = get_node_or_null(view_path);
    // Try casting to known views, or just set property if we want generic
    // For now, let's assume specific or usage of 'call'
    // But we have headers, so let's try cast
    IsometricView *iso_view = Object::cast_to<IsometricView>(node);
    if (iso_view) {
      iso_view->set_world_data(world_data);
    } else {
      // Try standard TileMapLayer property if we implemented it generic?
      // Currently TileMapLayer doesn't have set_world_data.
      // We can use call.
      if (node && node->has_method("set_world_data")) {
        node->call("set_world_data", world_data);
      }
    }
  }

  // Spawn Player
  if (player_scene.is_valid()) {
    Node *instance = player_scene->instantiate();
    Node2D *player = Object::cast_to<Node2D>(instance);

    if (player) {
      add_child(player);

      // Find a valid start point (random room center)
      // Ideally Generator should expose start point.
      // For now, let's find the first empty floor.
      Vector3i start_pos(1, 0, 1);

      // Allow Generator to expose rooms? Or search.
      // Search center
      for (int x = 0; x < map_width; ++x) {
        for (int z = 0; z < map_height; ++z) {
          if (world_data->get_voxel(Vector3i(x, 0, z)) == 0) { // Floor
            start_pos = Vector3i(x, 0, z);
            break;
          }
        }
        if (start_pos != Vector3i(1, 0, 1))
          break;
      }

      // Coordinate conversion needed?
      // UnviersalData (X, Z) -> Isometric (Screen)
      // Isometric mapping:
      // ScreenX = (mapX - mapZ) * TileWidth/2
      // ScreenY = (mapX + mapZ) * TileHeight/2
      // BUT wait, our IsometricView is a TileMapLayer. It handles mapping.
      // Node2D position is in PIXELS.
      // We need to know tile size. default 64x32?
      // Ideally we ask the View to map coord to position.

      // Simplification: Assume standard Iso setup or 2D grid.
      // Let's iterate:
      // If SideView -> x * tile_size, y * tile_size
      // If IsoView -> Iso transform

      // For this iteration, let's place it at (0,0) or let user handle spawn
      // via signal? "on_level_started(start_pos)"

      // Better: use map_to_local if linked to view

      Node *node = get_node_or_null(view_path);
      TileMapLayer *tilemap = Object::cast_to<TileMapLayer>(node);
      if (tilemap) {
        // UniversalData (x, z) maps to TileMap (x, y) ?
        // IsoView Logic: map (x, z) -> View (x, -z) ? Wait, IsoView
        // implementation: "Vector2i view_pos(p_pos.x, -p_pos.z);" line 124 in
        // iso_view.cpp wait, iso_view.cpp line 63: Vector2i view_pos(p_pos.x,
        // p_pos.z); << CHECK THIS SideView: Vector2i view_pos(p_pos.x,
        // -p_pos.z);

        // Let's assume IsoView uses (x, y) map coords directly.
        Vector2i tile_pos(start_pos.x, start_pos.z);
        player->set_position(tilemap->map_to_local(tile_pos));
      } else {
        player->set_position(Vector2(start_pos.x * 64, start_pos.z * 64));
      }
    }
  }
}

void RoguelikeManager::set_view_path(const NodePath &p_path) {
  view_path = p_path;
}

NodePath RoguelikeManager::get_view_path() const { return view_path; }

void RoguelikeManager::set_player_scene(const Ref<PackedScene> &p_scene) {
  player_scene = p_scene;
}

Ref<PackedScene> RoguelikeManager::get_player_scene() const {
  return player_scene;
}

void RoguelikeManager::set_map_width(int p_width) { map_width = p_width; }

int RoguelikeManager::get_map_width() const { return map_width; }

void RoguelikeManager::set_map_height(int p_height) { map_height = p_height; }

int RoguelikeManager::get_map_height() const { return map_height; }

void RoguelikeManager::set_room_count(int p_count) { room_count = p_count; }

int RoguelikeManager::get_room_count() const { return room_count; }
