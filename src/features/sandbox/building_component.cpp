#include "building_component.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

BuildingComponent::BuildingComponent() {}

BuildingComponent::~BuildingComponent() {}

void BuildingComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_world_data", "data"),
                       &BuildingComponent::set_world_data);
  ClassDB::bind_method(D_METHOD("get_world_data"),
                       &BuildingComponent::get_world_data);
  ClassDB::bind_method(D_METHOD("set_inventory", "inventory"),
                       &BuildingComponent::set_inventory);
  ClassDB::bind_method(D_METHOD("get_inventory"),
                       &BuildingComponent::get_inventory);
  ClassDB::bind_method(D_METHOD("set_range", "range"),
                       &BuildingComponent::set_range);
  ClassDB::bind_method(D_METHOD("get_range"), &BuildingComponent::get_range);
  ClassDB::bind_method(D_METHOD("set_default_block_id", "id"),
                       &BuildingComponent::set_default_block_id);
  ClassDB::bind_method(D_METHOD("get_default_block_id"),
                       &BuildingComponent::get_default_block_id);
  ClassDB::bind_method(D_METHOD("place_voxel", "global_pos", "item_id"),
                       &BuildingComponent::place_voxel, DEFVAL(-1));

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "world_data",
                            PROPERTY_HINT_RESOURCE_TYPE, "UniversalWorldData"),
               "set_world_data", "get_world_data");
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "inventory",
                            PROPERTY_HINT_RESOURCE_TYPE, "Inventory"),
               "set_inventory", "get_inventory");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range"), "set_range", "get_range");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "default_block_id"),
               "set_default_block_id", "get_default_block_id");
}

void BuildingComponent::set_world_data(const Ref<UniversalWorldData> &p_data) {
  world_data = p_data;
}
Ref<UniversalWorldData> BuildingComponent::get_world_data() const {
  return world_data;
}

void BuildingComponent::set_inventory(const Ref<Inventory> &p_inventory) {
  inventory = p_inventory;
}
Ref<Inventory> BuildingComponent::get_inventory() const { return inventory; }

void BuildingComponent::set_range(float p_range) { range = p_range; }
float BuildingComponent::get_range() const { return range; }

void BuildingComponent::set_default_block_id(int p_id) {
  default_block_id = p_id;
}
int BuildingComponent::get_default_block_id() const { return default_block_id; }

bool BuildingComponent::place_voxel(Vector2 global_pos, int item_id) {
  if (!world_data.is_valid())
    return false;

  // Check range
  if (get_global_position().distance_to(global_pos) > range) {
    return false;
  }

  // Determine ID to place
  int id_to_place = (item_id != -1) ? item_id : default_block_id;

  // Logic to consume form inventory?
  // If item_id is provided, check if inventory has it.
  // For now, assume strict mapping: Item ID == Voxel ID.
  // Ideally GameItem has property "voxel_id".
  // Let's assume infinite placement for Creative/Demo, or simple consume if
  // inventory is set.

  if (inventory.is_valid() && item_id != -1) {
    // Check if we have item with name/ID corresponding to this block?
    // This requires mapping. Skipped for initial prototype.
  }

  // Grid Mapping (Simple TILE_SIZE=32 assumption)
  const int TILE_SIZE = 32;
  Vector3i grid_pos;
  grid_pos.x = static_cast<int>(global_pos.x / TILE_SIZE);
  grid_pos.y = static_cast<int>(global_pos.y / TILE_SIZE);
  grid_pos.z = 0;

  int current_voxel = world_data->get_voxel(grid_pos);
  if (current_voxel == 0 || current_voxel == -1) { // Empty
    world_data->set_voxel(grid_pos, id_to_place);
    return true;
  }

  return false;
}
