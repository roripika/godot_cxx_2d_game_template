#ifndef BUILDING_COMPONENT_H
#define BUILDING_COMPONENT_H

#include "core/universal_world_data.h"
#include "items/inventory.h"
#include <godot_cpp/classes/node2d.hpp>

using namespace godot;

class BuildingComponent : public Node2D {
  GDCLASS(BuildingComponent, Node2D)

private:
  Ref<UniversalWorldData> world_data;
  Ref<Inventory> inventory;
  float range = 100.0;
  int default_block_id = 1; // Wall

protected:
  static void _bind_methods();

public:
  BuildingComponent();
  ~BuildingComponent();

  void set_world_data(const Ref<UniversalWorldData> &p_data);
  Ref<UniversalWorldData> get_world_data() const;

  void set_inventory(const Ref<Inventory> &p_inventory);
  Ref<Inventory> get_inventory() const;

  void set_range(float p_range);
  float get_range() const;

  void set_default_block_id(int p_id);
  int get_default_block_id() const;

  // Attempt to place voxel at global position
  bool place_voxel(Vector2 global_pos, int item_id = -1);
};

#endif // BUILDING_COMPONENT_H
