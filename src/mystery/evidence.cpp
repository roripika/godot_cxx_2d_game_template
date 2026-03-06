#include "evidence.h"

#include <godot_cpp/core/class_db.hpp>

namespace mystery {

void Evidence::_bind_methods() {
  // item_id
  ClassDB::bind_method(D_METHOD("set_item_id", "id"), &Evidence::set_item_id);
  ClassDB::bind_method(D_METHOD("get_item_id"), &Evidence::get_item_id);
  ADD_PROPERTY(godot::PropertyInfo(godot::Variant::STRING, "item_id"),
               "set_item_id", "get_item_id");

  // contradicts_with
  ClassDB::bind_method(D_METHOD("set_contradicts_with", "statement_id"),
                       &Evidence::set_contradicts_with);
  ClassDB::bind_method(D_METHOD("get_contradicts_with"),
                       &Evidence::get_contradicts_with);
  ADD_PROPERTY(
      godot::PropertyInfo(godot::Variant::STRING, "contradicts_with"),
      "set_contradicts_with", "get_contradicts_with");

  // is_key_item
  ClassDB::bind_method(D_METHOD("set_is_key_item", "value"),
                       &Evidence::set_is_key_item);
  ClassDB::bind_method(D_METHOD("get_is_key_item"), &Evidence::get_is_key_item);
  ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "is_key_item"),
               "set_is_key_item", "get_is_key_item");

  // ロジック
  ClassDB::bind_method(D_METHOD("can_contradict", "statement_id"),
                       &Evidence::can_contradict);
}

// ------------------------------------------------------------------
// プロパティ
// ------------------------------------------------------------------

void Evidence::set_item_id(const godot::String &id) { item_id_ = id; }
godot::String Evidence::get_item_id() const { return item_id_; }

void Evidence::set_contradicts_with(const godot::String &statement_id) {
  contradicts_with_ = statement_id;
}
godot::String Evidence::get_contradicts_with() const {
  return contradicts_with_;
}

void Evidence::set_is_key_item(bool value) { is_key_item_ = value; }
bool Evidence::get_is_key_item() const { return is_key_item_; }

// ------------------------------------------------------------------
// ゲームロジック
// ------------------------------------------------------------------

bool Evidence::can_contradict(const godot::String &statement_id) const {
  if (contradicts_with_.is_empty()) {
    return false;
  }
  return contradicts_with_ == statement_id;
}

} // namespace mystery
