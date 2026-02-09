#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include "core/universal_world_data.h"
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class WorldGenerator : public RefCounted {
  GDCLASS(WorldGenerator, RefCounted)

private:
  Ref<FastNoiseLite> noise;

protected:
  static void _bind_methods();

public:
  WorldGenerator();
  ~WorldGenerator();

  void generate_chunk(const Ref<UniversalWorldData> &p_world_data,
                      const Vector2i &p_chunk_pos);
};

#endif
