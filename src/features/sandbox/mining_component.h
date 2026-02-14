#ifndef MINING_COMPONENT_H
#define MINING_COMPONENT_H

#include "core/universal_world_data.h"
#include <godot_cpp/classes/node2d.hpp>

using namespace godot;

class MiningComponent : public Node2D {
  GDCLASS(MiningComponent, Node2D)

private:
  Ref<UniversalWorldData> world_data;
  float mining_power = 1.0;
  float range = 100.0;

protected:
  static void _bind_methods();

public:
  MiningComponent();
  ~MiningComponent();

  void set_world_data(const Ref<UniversalWorldData> &p_data);
  Ref<UniversalWorldData> get_world_data() const;

  void set_mining_power(float p_power);
  float get_mining_power() const;

  void set_range(float p_range);
  float get_range() const;

  // Attempt to mine at global position
  bool mine(Vector2 global_pos);
};

#endif // MINING_COMPONENT_H
