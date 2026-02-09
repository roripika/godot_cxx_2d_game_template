#include "side_scrolling_view.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SideScrollingView::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_world_data", "data"),
                       &SideScrollingView::set_world_data);
  ClassDB::bind_method(D_METHOD("get_world_data"),
                       &SideScrollingView::get_world_data);
  ClassDB::bind_method(D_METHOD("set_depth_layer", "depth"),
                       &SideScrollingView::set_depth_layer);
  ClassDB::bind_method(D_METHOD("get_depth_layer"),
                       &SideScrollingView::get_depth_layer);
  ClassDB::bind_method(D_METHOD("update_visuals"),
                       &SideScrollingView::update_visuals);
  ClassDB::bind_method(D_METHOD("_on_voxel_changed", "pos"),
                       &SideScrollingView::_on_voxel_changed);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "world_data",
                            PROPERTY_HINT_RESOURCE_TYPE, "UniversalWorldData"),
               "set_world_data", "get_world_data");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "depth_layer"), "set_depth_layer",
               "get_depth_layer");
}

SideScrollingView::SideScrollingView() { depth_layer = 0; }

SideScrollingView::~SideScrollingView() {
  if (world_data.is_valid()) {
    world_data->disconnect("voxel_changed",
                           Callable(this, "_on_voxel_changed"));
  }
}

void SideScrollingView::set_world_data(const Ref<UniversalWorldData> &p_data) {
  if (world_data.is_valid()) {
    world_data->disconnect("voxel_changed",
                           Callable(this, "_on_voxel_changed"));
  }

  world_data = p_data;

  if (world_data.is_valid()) {
    world_data->connect("voxel_changed", Callable(this, "_on_voxel_changed"));
    update_visuals();
  }
}

Ref<UniversalWorldData> SideScrollingView::get_world_data() const {
  return world_data;
}

void SideScrollingView::set_depth_layer(int p_depth) {
  depth_layer = p_depth;
  update_visuals();
}

int SideScrollingView::get_depth_layer() const { return depth_layer; }

void SideScrollingView::_on_voxel_changed(const Vector3i &p_pos) {
  // Check if the change happened in the current depth layer
  // World Y -> Depth Layer
  if (p_pos.y == depth_layer) {
    if (world_data.is_valid()) {
      int id = world_data->get_voxel(p_pos);

      // Map World (X, Z) to View (X, -Z)
      // Godot 2D: +Y is Down. World Z is Up. So View Y = -World Z.
      Vector2i view_pos(p_pos.x, -p_pos.z);

      if (id >= 0) {
        set_cell(view_pos, 0, Vector2i(id, 0));
      } else {
        erase_cell(view_pos);
      }
    }
  }
}

void SideScrollingView::update_visuals() {
  clear();
  if (world_data.is_null())
    return;

  Vector3i size = world_data->get_size();

  // Check valid depth
  if (depth_layer >= 0 && depth_layer < size.y) {
    for (int z = 0; z < size.z; ++z) {
      for (int x = 0; x < size.x; ++x) {
        // Fixed Y (depth), iterate X and Z
        int id = world_data->get_voxel(Vector3i(x, depth_layer, z));

        if (id >= 0) {
          // Map World (X, Z) to View (X, -Z)
          Vector2i view_pos(x, -z);
          set_cell(view_pos, 0, Vector2i(id, 0));
        }
      }
    }
  }
}
