#include "roguelike_generator.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void RoguelikeGenerator::_bind_methods() {
  ClassDB::bind_method(D_METHOD("generate_dungeon", "world", "width", "height",
                                "depth_layer", "room_count"),
                       &RoguelikeGenerator::generate_dungeon);
}

RoguelikeGenerator::RoguelikeGenerator() {}

RoguelikeGenerator::~RoguelikeGenerator() {}

void RoguelikeGenerator::_create_room(Ref<UniversalWorldData> world,
                                      const Rect2i &rect, int floor_id,
                                      int wall_id, int depth_layer) {
  for (int x = rect.position.x; x < rect.position.x + rect.size.x; ++x) {
    for (int z = rect.position.y; z < rect.position.y + rect.size.y; ++z) {
      world->set_voxel(Vector3i(x, depth_layer, z), 0);
    }
  }
}

void RoguelikeGenerator::_create_h_tunnel(Ref<UniversalWorldData> world, int x1,
                                          int x2, int z, int floor_id,
                                          int wall_id, int depth_layer) {
  int start = (x1 < x2) ? x1 : x2;
  int end = (x1 < x2) ? x2 : x1;

  for (int x = start; x <= end; ++x) {
    world->set_voxel(Vector3i(x, depth_layer, z), 0);
  }
}

void RoguelikeGenerator::_create_v_tunnel(Ref<UniversalWorldData> world, int z1,
                                          int z2, int x, int floor_id,
                                          int wall_id, int depth_layer) {
  int start = (z1 < z2) ? z1 : z2;
  int end = (z1 < z2) ? z2 : z1;

  for (int z = start; z <= end; ++z) {
    world->set_voxel(Vector3i(x, depth_layer, z), 0);
  }
}

void RoguelikeGenerator::generate_dungeon(Ref<UniversalWorldData> world,
                                          int map_width, int map_height,
                                          int depth_layer, int room_count) {
  if (world.is_null())
    return;

  UtilityFunctions::print("Generating Dungeon...");

  for (int x = 0; x < map_width; ++x) {
    for (int z = 0; z < map_height; ++z) {
      world->set_voxel(Vector3i(x, depth_layer, z), 1);
    }
  }

  rooms.clear();

  for (int i = 0; i < room_count; ++i) {
    int w = UtilityFunctions::randi_range(6, 15);
    int h = UtilityFunctions::randi_range(6, 15);
    int x = UtilityFunctions::randi_range(1, map_width - w - 1);
    int z = UtilityFunctions::randi_range(1, map_height - h - 1);

    Rect2i new_rect(x, z, w, h);
    bool intersects = false;

    for (int j = 0; j < rooms.size(); ++j) {
      if (new_rect.intersects(rooms[j].rect)) {
        intersects = true;
        break;
      }
    }

    if (!intersects) {
      _create_room(world, new_rect, 0, 1, depth_layer);

      Vector3i new_center(x + w / 2, depth_layer, z + h / 2);

      if (rooms.size() > 0) {
        Vector3i prev_center = rooms[rooms.size() - 1].center;

        if (UtilityFunctions::randi() % 2 == 1) {
          _create_h_tunnel(world, prev_center.x, new_center.x, prev_center.z, 0,
                           1, depth_layer);
          _create_v_tunnel(world, prev_center.z, new_center.z, new_center.x, 0,
                           1, depth_layer);
        } else {
          _create_v_tunnel(world, prev_center.z, new_center.z, prev_center.x, 0,
                           1, depth_layer);
          _create_h_tunnel(world, prev_center.x, new_center.x, new_center.z, 0,
                           1, depth_layer);
        }
      }

      Room room;
      room.rect = new_rect;
      room.center = new_center;
      rooms.push_back(room);
    }
  }

  UtilityFunctions::print("Dungeon Generated with ", rooms.size(), " rooms.");
}
