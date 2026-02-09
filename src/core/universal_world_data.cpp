#include "universal_world_data.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void UniversalWorldData::_bind_methods() {
  ClassDB::bind_method(D_METHOD("initialize", "size"),
                       &UniversalWorldData::initialize);
  ClassDB::bind_method(D_METHOD("set_voxel", "pos", "id"),
                       &UniversalWorldData::set_voxel);
  ClassDB::bind_method(D_METHOD("get_voxel", "pos"),
                       &UniversalWorldData::get_voxel);
  ClassDB::bind_method(D_METHOD("get_size"), &UniversalWorldData::get_size);

  ADD_SIGNAL(
      MethodInfo("voxel_changed", PropertyInfo(Variant::VECTOR3I, "pos")));
}

UniversalWorldData::UniversalWorldData() : size(Vector3i(0, 0, 0)) {}

UniversalWorldData::~UniversalWorldData() {}

void UniversalWorldData::initialize(const Vector3i &p_size) {
  size = p_size;
  data.resize(size.x * size.y * size.z, -1);
}

void UniversalWorldData::set_voxel(const Vector3i &p_pos, int p_id) {
  if (p_pos.x < 0 || p_pos.x >= size.x || p_pos.y < 0 || p_pos.y >= size.y ||
      p_pos.z < 0 || p_pos.z >= size.z) {
    return;
  }
  int index = p_pos.x + size.x * (p_pos.y + size.y * p_pos.z);

  if (index >= 0 && index < data.size()) {
    if (data[index] != p_id) {
      data[index] = p_id;
      emit_signal("voxel_changed", p_pos);
    }
  }
}

int UniversalWorldData::get_voxel(const Vector3i &p_pos) const {
  if (p_pos.x < 0 || p_pos.x >= size.x || p_pos.y < 0 || p_pos.y >= size.y ||
      p_pos.z < 0 || p_pos.z >= size.z) {
    return -1;
  }
  int index = p_pos.x + size.x * (p_pos.y + size.y * p_pos.z);
  if (index >= 0 && index < data.size()) {
    return data[index];
  }
  return -1;
}
