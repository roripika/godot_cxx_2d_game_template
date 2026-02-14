#include "mining_component.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

MiningComponent::MiningComponent() {}

MiningComponent::~MiningComponent() {}

void MiningComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_world_data", "data"),
                       &MiningComponent::set_world_data);
  ClassDB::bind_method(D_METHOD("get_world_data"),
                       &MiningComponent::get_world_data);
  ClassDB::bind_method(D_METHOD("set_mining_power", "power"),
                       &MiningComponent::set_mining_power);
  ClassDB::bind_method(D_METHOD("get_mining_power"),
                       &MiningComponent::get_mining_power);
  ClassDB::bind_method(D_METHOD("set_range", "range"),
                       &MiningComponent::set_range);
  ClassDB::bind_method(D_METHOD("get_range"), &MiningComponent::get_range);
  ClassDB::bind_method(D_METHOD("mine", "global_pos"), &MiningComponent::mine);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "world_data",
                            PROPERTY_HINT_RESOURCE_TYPE, "UniversalWorldData"),
               "set_world_data", "get_world_data");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mining_power"), "set_mining_power",
               "get_mining_power");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range"), "set_range", "get_range");

  ADD_SIGNAL(MethodInfo("mined", PropertyInfo(Variant::VECTOR2, "pos"),
                        PropertyInfo(Variant::INT, "item_id")));
}

void MiningComponent::set_world_data(const Ref<UniversalWorldData> &p_data) {
  world_data = p_data;
}
Ref<UniversalWorldData> MiningComponent::get_world_data() const {
  return world_data;
}

void MiningComponent::set_mining_power(float p_power) {
  mining_power = p_power;
}
float MiningComponent::get_mining_power() const { return mining_power; }

void MiningComponent::set_range(float p_range) { range = p_range; }
float MiningComponent::get_range() const { return range; }

bool MiningComponent::mine(Vector2 global_pos) {
  if (!world_data.is_valid())
    return false;

  // Check range
  if (get_global_position().distance_to(global_pos) > range) {
    return false;
  }

  // Convert global pos to grid pos
  // Assuming 1 unit = 1 pixel for now, but usually tiles are 16x16 or similar.
  // Sandbox usually implies grid based.
  // UniversalWorldData assumes 3D grid, but we are 2D view.
  // Need mapping strategy. For now, assume simple mapping: x, y -> x, y, z=0
  // But tile size? IsometricView uses 64x32. SideScrollingView uses ?
  // Let's assume passed pos is already converted or we use a standard grid
  // size. For universal, we should probably have a `GridMapper` or similar, or
  // just assume input is "World Coordinate" and let WorldData logic handle
  // conversion if it had that info. Since WorldData is just data, the View
  // usually handles conversion. But Component needs to know which voxel. Let's
  // assume input is Global Position and we use a fixed TILE_SIZE for now,
  // say 32. Or simpler: WorldData is passed integer coordinates? No,
  // implementation calls for Vector2 global_pos.

  // HACK: Hardcoding Tile Size 32 for now as placeholder for Sandbox 2D
  const int TILE_SIZE = 32;
  Vector3i grid_pos;
  grid_pos.x = static_cast<int>(global_pos.x / TILE_SIZE);
  grid_pos.y = static_cast<int>(global_pos.y / TILE_SIZE);
  grid_pos.z = 0; // Default layer

  int voxel_id = world_data->get_voxel(grid_pos);
  if (voxel_id > 0) {                   // If not empty
    world_data->set_voxel(grid_pos, 0); // Remove
    emit_signal("mined", global_pos,
                voxel_id); // Emit signal to spawn loot or add to inventory
    return true;
  }

  return false;
}
