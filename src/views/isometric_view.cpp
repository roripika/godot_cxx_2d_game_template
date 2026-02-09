#include "isometric_view.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void IsometricView::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_world_data", "data"),
                       &IsometricView::set_world_data);
  ClassDB::bind_method(D_METHOD("get_world_data"),
                       &IsometricView::get_world_data);
  ClassDB::bind_method(D_METHOD("set_voxel", "pos", "id"),
                       &IsometricView::set_voxel);
  ClassDB::bind_method(D_METHOD("get_voxel", "pos"), &IsometricView::get_voxel);
  ClassDB::bind_method(D_METHOD("map_to_local_custom", "pos"),
                       &IsometricView::map_to_local_custom);
  ClassDB::bind_method(D_METHOD("update_visuals"),
                       &IsometricView::update_visuals);
  ClassDB::bind_method(D_METHOD("_on_voxel_changed", "pos"),
                       &IsometricView::_on_voxel_changed);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "world_data",
                            PROPERTY_HINT_RESOURCE_TYPE, "UniversalWorldData"),
               "set_world_data", "get_world_data");
}

IsometricView::IsometricView() {}

IsometricView::~IsometricView() {
  if (world_data.is_valid()) {
    world_data->disconnect("voxel_changed",
                           Callable(this, "_on_voxel_changed"));
  }
}

void IsometricView::set_world_data(const Ref<UniversalWorldData> &p_data) {
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

Ref<UniversalWorldData> IsometricView::get_world_data() const {
  return world_data;
}

void IsometricView::set_voxel(const Vector2i &p_pos, int p_id) {
  if (world_data.is_valid()) {
    // Map 2D Iso click to 3D world (assume z=0)
    world_data->set_voxel(Vector3i(p_pos.x, p_pos.y, 0), p_id);
  }
}

int IsometricView::get_voxel(const Vector2i &p_pos) const {
  if (world_data.is_valid()) {
    return world_data->get_voxel(Vector3i(p_pos.x, p_pos.y, 0));
  }
  return -1;
}

void IsometricView::_on_voxel_changed(const Vector3i &p_pos) {
  if (p_pos.z == 0) {
    if (world_data.is_valid()) {
      int id = world_data->get_voxel(p_pos);
      if (id >= 0) {
        set_cell(Vector2i(p_pos.x, p_pos.y), 0, Vector2i(id, 0));
      } else {
        erase_cell(Vector2i(p_pos.x, p_pos.y));
      }
    }
  }
}

void IsometricView::update_visuals() {
  clear();
  if (world_data.is_null())
    return;

  Vector3i size = world_data->get_size();

  // Only render z=0 for now
  if (size.z > 0) {
    for (int y = 0; y < size.y; ++y) {
      for (int x = 0; x < size.x; ++x) {
        int id = world_data->get_voxel(Vector3i(x, y, 0));
        if (id >= 0) {
          set_cell(Vector2i(x, y), 0, Vector2i(id, 0));
        }
      }
    }
  }
}

Vector2 IsometricView::map_to_local_custom(const Vector2i &p_pos) const {
  Ref<TileSet> ts = get_tile_set();
  if (ts.is_null()) {
    return Vector2();
  }

  Size2i tile_size = ts->get_tile_size();
  float half_w = tile_size.x * 0.5f;
  float half_h = tile_size.y * 0.5f;

  return Vector2((p_pos.x - p_pos.y) * half_w, (p_pos.x + p_pos.y) * half_h);
}
