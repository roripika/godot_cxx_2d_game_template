#include "services/item_service.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

ItemService *ItemService::singleton_ = nullptr;

ItemService::ItemService() {
  if (singleton_ == nullptr) {
    singleton_ = this;
  }
}

ItemService::~ItemService() {
  if (singleton_ == this) {
    singleton_ = nullptr;
  }
}

ItemService *ItemService::get_singleton() { return singleton_; }

void ItemService::_bind_methods() {
  // マスターカタログ
  ClassDB::bind_method(D_METHOD("load_master_data", "path"),
                       &ItemService::load_master_data);
  ClassDB::bind_method(D_METHOD("register_item", "id", "data"),
                       &ItemService::register_item);
  ClassDB::bind_method(D_METHOD("get_item_data", "id"),
                       &ItemService::get_item_data);
  ClassDB::bind_method(D_METHOD("is_known_item", "id"),
                       &ItemService::is_known_item);

  // インベントリ
  ClassDB::bind_method(D_METHOD("add_item", "id"), &ItemService::add_item);
  ClassDB::bind_method(D_METHOD("remove_item", "id"), &ItemService::remove_item);
  ClassDB::bind_method(D_METHOD("has_item", "id"), &ItemService::has_item);
  ClassDB::bind_method(D_METHOD("get_inventory"), &ItemService::get_inventory);
  ClassDB::bind_method(D_METHOD("get_inventory_data"),
                       &ItemService::get_inventory_data);
  ClassDB::bind_method(D_METHOD("clear_inventory"),
                       &ItemService::clear_inventory);

  // シリアライズ
  ClassDB::bind_method(D_METHOD("serialize"), &ItemService::serialize);
  ClassDB::bind_method(D_METHOD("deserialize", "data"),
                       &ItemService::deserialize);

  // シグナル
  ADD_SIGNAL(MethodInfo("item_added",
                        PropertyInfo(Variant::STRING, "id")));
  ADD_SIGNAL(MethodInfo("item_removed",
                        PropertyInfo(Variant::STRING, "id")));
}

// ------------------------------------------------------------------
// マスターカタログ
// ------------------------------------------------------------------

void ItemService::load_master_data(const godot::String &path) {
  if (!godot::FileAccess::file_exists(path)) {
    godot::UtilityFunctions::push_warning(
        "ItemService: master data file not found: " + path);
    return;
  }

  auto fa = godot::FileAccess::open(path, godot::FileAccess::READ);
  if (!fa.is_valid()) {
    godot::UtilityFunctions::push_warning(
        "ItemService: failed to open master data: " + path);
    return;
  }

  godot::String json_text = fa->get_as_text();
  fa->close();

  godot::JSON json_parser;
  godot::Error err = json_parser.parse(json_text);
  if (err != godot::OK) {
    godot::UtilityFunctions::push_warning(
        "ItemService: JSON parse error in " + path);
    return;
  }

  godot::Variant parsed = json_parser.get_data();
  if (parsed.get_type() != godot::Variant::ARRAY) {
    godot::UtilityFunctions::push_warning(
        "ItemService: master data must be a JSON array: " + path);
    return;
  }

  godot::Array items = parsed;
  for (int i = 0; i < items.size(); ++i) {
    if (items[i].get_type() != godot::Variant::DICTIONARY) {
      continue;
    }
    godot::Dictionary entry = items[i];
    if (!entry.has("id")) {
      godot::UtilityFunctions::push_warning(
          "ItemService: item entry missing 'id' field, skipped.");
      continue;
    }
    godot::String id = entry["id"];
    item_master_[id] = entry;
  }

  godot::UtilityFunctions::print(
      godot::String("ItemService: loaded {0} items from {1}")
          .format(godot::Array::make(item_master_.size(), path)));
}

void ItemService::register_item(const godot::String &id,
                                const godot::Dictionary &data) {
  item_master_[id] = data;
}

godot::Dictionary ItemService::get_item_data(const godot::String &id) const {
  if (item_master_.has(id)) {
    return item_master_[id];
  }
  return godot::Dictionary();
}

bool ItemService::is_known_item(const godot::String &id) const {
  return item_master_.has(id);
}

// ------------------------------------------------------------------
// インベントリ
// ------------------------------------------------------------------

bool ItemService::add_item(const godot::String &id) {
  if (inventory_.has(id)) {
    return false; // 既に所持
  }
  inventory_.append(id);
  emit_signal("item_added", id);
  return true;
}

bool ItemService::remove_item(const godot::String &id) {
  int idx = inventory_.find(id);
  if (idx < 0) {
    return false;
  }
  inventory_.remove_at(idx);
  emit_signal("item_removed", id);
  return true;
}

bool ItemService::has_item(const godot::String &id) const {
  return inventory_.has(id);
}

godot::Array ItemService::get_inventory() const { return inventory_; }

godot::Array ItemService::get_inventory_data() const {
  godot::Array result;
  for (int i = 0; i < inventory_.size(); ++i) {
    godot::String id = inventory_[i];
    godot::Dictionary entry = get_item_data(id);
    if (entry.is_empty()) {
      // マスター未登録のアイテムは最低限 id だけ返す
      godot::Dictionary fallback;
      fallback["id"] = id;
      result.append(fallback);
    } else {
      result.append(entry);
    }
  }
  return result;
}

void ItemService::clear_inventory() {
  inventory_.clear();
}

// ------------------------------------------------------------------
// シリアライズ
// ------------------------------------------------------------------

godot::Dictionary ItemService::serialize() const {
  godot::Dictionary d;
  d["inventory"] = inventory_.duplicate();
  return d;
}

void ItemService::deserialize(const godot::Dictionary &data) {
  if (data.has("inventory")) {
    godot::Variant v = data["inventory"];
    if (v.get_type() == godot::Variant::ARRAY) {
      inventory_ = godot::Array(v).duplicate();
    }
  }
}

} // namespace karakuri
