#include "world_generator.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

WorldGenerator::WorldGenerator() {
  noise.instantiate();
  noise->set_seed(12345);
  noise->set_frequency(0.1);
}

WorldGenerator::~WorldGenerator() {}

void WorldGenerator::_bind_methods() {
  ClassDB::bind_method(D_METHOD("generate_chunk", "world_data", "chunk_pos"),
                       &WorldGenerator::generate_chunk);
}

void WorldGenerator::generate_chunk(const Ref<UniversalWorldData> &p_world_data,
                                    const Vector2i &p_chunk_pos) {
  if (p_world_data.is_null()) {
    UtilityFunctions::printerr("WorldGenerator: UniversalWorldData is null.");
    return;
  }

  // Simplified generation: fill a 10x10 area at the chunk position offset
  int chunk_size = 10;
  Vector2i base_pos = p_chunk_pos * chunk_size;

  for (int x = 0; x < chunk_size; ++x) {
    for (int y = 0; y < chunk_size; ++y) {
      Vector2i current_pos_2d = base_pos + Vector2i(x, y);

      // Use noise to decide tile
      float noise_val = noise->get_noise_2d(current_pos_2d.x, current_pos_2d.y);

      // Write to Z=0
      Vector3i current_pos_3d(current_pos_2d.x, current_pos_2d.y, 0);

      if (noise_val > 0.0) {
        // Place a tile (ID 0)
        p_world_data->set_voxel(current_pos_3d, 0);
      } else {
        // Empty space (or clear it)
        p_world_data->set_voxel(current_pos_3d, -1);
      }
    }
  }
}
