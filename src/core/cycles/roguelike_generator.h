#ifndef ROGUELIKE_GENERATOR_H
#define ROGUELIKE_GENERATOR_H

#include "../universal_world_data.h"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/vector3i.hpp>

using namespace godot;

class RoguelikeGenerator : public RefCounted {
  GDCLASS(RoguelikeGenerator, RefCounted)

private:
  struct Room {
    Rect2i rect;
    Vector3i center;
  };

  Vector<Room> rooms;

  void _create_room(Ref<UniversalWorldData> world, const Rect2i &rect,
                    int floor_id, int wall_id, int depth_layer);
  void _create_h_tunnel(Ref<UniversalWorldData> world, int x1, int x2, int y,
                        int floor_id, int wall_id, int depth_layer);
  void _create_v_tunnel(Ref<UniversalWorldData> world, int y1, int y2, int x,
                        int floor_id, int wall_id, int depth_layer);

protected:
  static void _bind_methods();

public:
  RoguelikeGenerator();
  ~RoguelikeGenerator();

  // Generates a simple dungeon on a specific depth layer (Y-level)
  void generate_dungeon(Ref<UniversalWorldData> world, int map_width,
                        int map_height, int depth_layer, int room_count);
};

#endif
