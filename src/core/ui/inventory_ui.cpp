#include "inventory_ui.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

InventoryUI::InventoryUI() { set_anchors_preset(Control::PRESET_FULL_RECT); }

InventoryUI::~InventoryUI() {}

void InventoryUI::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_inventory", "inventory"),
                       &InventoryUI::set_inventory);
  ClassDB::bind_method(D_METHOD("get_inventory"), &InventoryUI::get_inventory);
  ClassDB::bind_method(D_METHOD("refresh"), &InventoryUI::refresh);
  ClassDB::bind_method(D_METHOD("_on_slot_clicked", "index", "button_index"),
                       &InventoryUI::_on_slot_clicked);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "inventory",
                            PROPERTY_HINT_RESOURCE_TYPE, "Inventory"),
               "set_inventory", "get_inventory");
}

void InventoryUI::_ready() {
  if (!grid_container) {
    grid_container = memnew(GridContainer);
    grid_container->set_columns(columns);
    grid_container->set_anchors_preset(Control::PRESET_CENTER);
    add_child(grid_container);
  }
}

void InventoryUI::set_inventory(const Ref<Inventory> &p_inventory) {
  inventory = p_inventory;
  refresh();
}

Ref<Inventory> InventoryUI::get_inventory() const { return inventory; }

void InventoryUI::refresh() {
  if (!inventory.is_valid() || !grid_container)
    return;

  // Clear existing slots
  // Note: inefficient for large inventories, but fine for demo.
  // Ideally, re-use slots.
  while (grid_container->get_child_count() > 0) {
    Node *child = grid_container->get_child(0);
    grid_container->remove_child(child);
    child->queue_free(); // Safe deletion
  }

  TypedArray<Dictionary> slots = inventory->get_slots();
  int capacity = inventory->get_capacity();

  // Fill grid
  for (int i = 0; i < capacity; i++) {
    InventorySlotUI *slot_ui = memnew(InventorySlotUI);
    grid_container->add_child(slot_ui);
    slot_ui->set_slot_index(i);
    slot_ui->connect("slot_clicked", Callable(this, "_on_slot_clicked"));

    // Get item data from dictionary if exists
    if (i < slots.size()) {
      Dictionary slot = slots[i];
      if (slot.has("item")) {
        Ref<GameItem> item = slot["item"];
        int amount = slot.get("amount", 1);
        slot_ui->set_slot_data(item, amount);
      }
    }
  }
}

void InventoryUI::_on_slot_clicked(int index, int button_index) {
  UtilityFunctions::print("Slot clicked: ", index, " Button: ", button_index);
  // Logic for usage or drag would go here
  // For now just print
  if (inventory.is_valid()) {
    TypedArray<Dictionary> slots = inventory->get_slots();
    if (index < slots.size()) {
      Dictionary slot = slots[index];
      if (slot.has("item")) {
        Ref<GameItem> item = slot["item"];
        if (item.is_valid()) {
          UtilityFunctions::print("Item: ", item->get_item_name());
        }
      }
    }
  }
}

void InventoryUI::_notification(int p_what) {
  //
}
