#include "state_snapshot.h"

#include "../services/flag_service.h"
#include "../services/item_service.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace karakuri {

void StateSnapshot::_bind_methods() {
  ClassDB::bind_method(D_METHOD("restore_services"), &StateSnapshot::restore_services);

  ClassDB::bind_method(D_METHOD("get_scene_id"), &StateSnapshot::get_scene_id);
  ClassDB::bind_method(D_METHOD("set_scene_id", "id"), &StateSnapshot::set_scene_id);
  ClassDB::bind_method(D_METHOD("get_command_index"), &StateSnapshot::get_command_index);
  ClassDB::bind_method(D_METHOD("set_command_index", "index"), &StateSnapshot::set_command_index);
  ClassDB::bind_method(D_METHOD("get_flags"), &StateSnapshot::get_flags);
  ClassDB::bind_method(D_METHOD("get_inventory"), &StateSnapshot::get_inventory);

  ADD_PROPERTY(PropertyInfo(Variant::STRING, "scene_id"), "set_scene_id", "get_scene_id");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "command_index"), "set_command_index", "get_command_index");
}

// ------------------------------------------------------------------
// ファクトリ
// ------------------------------------------------------------------

Ref<StateSnapshot> StateSnapshot::capture(const String &scene_id,
                                          int action_index) {
  Ref<StateSnapshot> snap = memnew(StateSnapshot);

  FlagService *fs = FlagService::get_singleton();
  if (fs != nullptr) {
    snap->flags_ = fs->serialize().duplicate(true);
  }

  ItemService *is = ItemService::get_singleton();
  if (is != nullptr) {
    snap->inventory_ = is->get_inventory().duplicate(true);
  }

  snap->current_scene_id_ = scene_id;
  snap->current_command_index_ = action_index;

  return snap;
}

// ------------------------------------------------------------------
// 復元
// ------------------------------------------------------------------

void StateSnapshot::restore_services() const {
  FlagService *fs = FlagService::get_singleton();
  if (fs != nullptr) {
    fs->deserialize(flags_);
  }

  ItemService *is = ItemService::get_singleton();
  if (is != nullptr) {
    // ItemService::deserialize は {"inventory": [...]} 形式を期待する
    Dictionary inv_wrap;
    inv_wrap["inventory"] = inventory_.duplicate(true);
    is->deserialize(inv_wrap);
  }
}

// ------------------------------------------------------------------
// ゲッター / セッター
// ------------------------------------------------------------------

String StateSnapshot::get_scene_id() const { return current_scene_id_; }
int StateSnapshot::get_command_index() const { return current_command_index_; }
Dictionary StateSnapshot::get_flags() const { return flags_; }
Array StateSnapshot::get_inventory() const { return inventory_; }

void StateSnapshot::set_scene_id(const String &id) { current_scene_id_ = id; }
void StateSnapshot::set_command_index(int index) { current_command_index_ = index; }

} // namespace karakuri
