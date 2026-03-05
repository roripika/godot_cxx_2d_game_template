#ifndef SIDE_SCROLLING_VIEW_H
#define SIDE_SCROLLING_VIEW_H

#include "../universal_world_data.h"
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/tile_set.hpp>

namespace karakuri {

using namespace godot;

// Renders the world data as a side-scrolling platformer view.
class SideScrollingView : public TileMapLayer {
  GDCLASS(SideScrollingView, TileMapLayer)

private:
  Ref<karakuri::UniversalWorldData> world_data;
  int depth_layer; // Which Y-slice of the world to render (Depth)

protected:
  static void _bind_methods();
  void _on_voxel_changed(const Vector3i &p_pos);

public:
  SideScrollingView();
  ~SideScrollingView();

  void set_world_data(const Ref<karakuri::UniversalWorldData> &p_data);
  Ref<karakuri::UniversalWorldData> get_world_data() const;

  void set_depth_layer(int p_depth);
  int p_depth_layer() const; // Renamed to avoid getter conflict if necessary,
                             // but keep set/get naming

  int get_depth_layer() const;

  void update_visuals();
};

} // namespace karakuri

#endif
