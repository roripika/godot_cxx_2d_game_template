#ifndef ISOMETRIC_VIEW_H
#define ISOMETRIC_VIEW_H

#include "../core/universal_world_data.h"
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/tile_set.hpp>

using namespace godot;

class IsometricView : public TileMapLayer {
  GDCLASS(IsometricView, TileMapLayer)

private:
  Ref<UniversalWorldData> world_data;

protected:
  static void _bind_methods();

  // Signal callback
  void _on_voxel_changed(const Vector3i &p_pos);

public:
  IsometricView();
  ~IsometricView();

  void set_world_data(const Ref<UniversalWorldData> &p_data);
  Ref<UniversalWorldData> get_world_data() const;

  // Helper to interact with the view directly (proxies to data)
  // 2D Interface for Godot Editor/Scripts
  void set_voxel(const Vector2i &p_pos, int p_id);
  int get_voxel(const Vector2i &p_pos) const;

  // Manual update of visual tiles
  void update_visuals();

  // Optimized math
  Vector2 map_to_local_custom(const Vector2i &p_pos) const;
};

#endif
