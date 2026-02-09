#ifndef SIDE_SCROLLING_VIEW_H
#define SIDE_SCROLLING_VIEW_H

#include "../core/universal_world_data.h"
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/tile_set.hpp>

using namespace godot;

// Renders the world data as a side-scrolling platformer view.
// - World X -> View X
// - World Z (Height) -> View -Y (Up is negative Y in Godot 2D)
// - World Y (Depth) -> Layer index (default 0)
//
// This allows generating a "mountain" in world data (high Z) to be rendered as
// a high platform in Side View.
class SideScrollingView : public TileMapLayer {
  GDCLASS(SideScrollingView, TileMapLayer)

private:
  Ref<UniversalWorldData> world_data;
  int depth_layer; // Which Y-slice of the world to render (Depth)

protected:
  static void _bind_methods();
  void _on_voxel_changed(const Vector3i &p_pos);

public:
  SideScrollingView();
  ~SideScrollingView();

  void set_world_data(const Ref<UniversalWorldData> &p_data);
  Ref<UniversalWorldData> get_world_data() const;

  void set_depth_layer(int p_depth);
  int get_depth_layer() const;

  void update_visuals();
};

#endif
