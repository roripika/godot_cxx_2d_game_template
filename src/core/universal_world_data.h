#ifndef UNIVERSAL_WORLD_DATA_H
#define UNIVERSAL_WORLD_DATA_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <vector>

using namespace godot;

class UniversalWorldData : public Resource {
  GDCLASS(UniversalWorldData, Resource)

private:
  // Flat 3D array. Index = x + size.x * (y + size.y * z)
  std::vector<int> data;
  Vector3i size;

protected:
  static void _bind_methods();

public:
  UniversalWorldData();
  ~UniversalWorldData();

  // Initialize with size. Fills with -1.
  void initialize(const Vector3i &p_size);

  void set_voxel(const Vector3i &p_pos, int p_id);
  int get_voxel(const Vector3i &p_pos) const;

  Vector3i get_size() const { return size; }
};

#endif
