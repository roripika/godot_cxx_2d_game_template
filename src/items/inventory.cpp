#include "inventory.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

void Inventory::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_capacity", "capacity"),
                       &Inventory::set_capacity);
  ClassDB::bind_method(D_METHOD("get_capacity"), &Inventory::get_capacity);
  ClassDB::bind_method(D_METHOD("set_slots", "slots"), &Inventory::set_slots);
  ClassDB::bind_method(D_METHOD("get_slots"), &Inventory::get_slots);

  ClassDB::bind_method(D_METHOD("add_item", "item", "amount"),
                       &Inventory::add_item, DEFVAL(1));
  ClassDB::bind_method(D_METHOD("remove_item", "item", "amount"),
                       &Inventory::remove_item, DEFVAL(1));
  ClassDB::bind_method(D_METHOD("has_item", "item", "amount"),
                       &Inventory::has_item, DEFVAL(1));
  ClassDB::bind_method(D_METHOD("clear"), &Inventory::clear);

  ADD_PROPERTY(PropertyInfo(Variant::INT, "capacity"), "set_capacity",
               "get_capacity");
  ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "slots", PROPERTY_HINT_NONE, "",
                            PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_STORAGE),
               "set_slots", "get_slots");

  ADD_SIGNAL(MethodInfo("inventory_changed"));
}

Inventory::Inventory() {
  capacity = 20;
  // Initialize empty slots
  slots.resize(capacity);
  for (int i = 0; i < capacity; ++i) {
    Dictionary slot;
    slot["item"] = Variant(); // Null
    slot["amount"] = 0;
    slots[i] = slot;
  }
}

Inventory::~Inventory() {}

void Inventory::set_capacity(int p_capacity) {
  capacity = p_capacity;
  slots.resize(capacity);
  // Ensure new slots are initialized if grown
  for (int i = 0; i < capacity; ++i) {
    if (slots[i].get_type() == Variant::NIL) {
      Dictionary slot;
      slot["item"] = Variant();
      slot["amount"] = 0;
      slots[i] = slot;
    }
  }
  emit_signal("inventory_changed");
}

int Inventory::get_capacity() const { return capacity; }

void Inventory::set_slots(const TypedArray<Dictionary> &p_slots) {
  slots = p_slots;
  capacity = slots.size();
  emit_signal("inventory_changed");
}

TypedArray<Dictionary> Inventory::get_slots() const { return slots; }

bool Inventory::add_item(const Ref<GameItem> &p_item, int p_amount) {
  if (p_item.is_null() || p_amount <= 0)
    return false;

  // 1. Try to stack
  if (p_item->is_stackable()) {
    for (int i = 0; i < slots.size(); ++i) {
      Dictionary slot = slots[i];
      Ref<GameItem> slot_item = slot.get("item", Variant());

      if (slot_item.is_valid() && slot_item == p_item) {
        int current_amount = slot["amount"];
        int max_stack = p_item->get_max_stack();

        if (current_amount < max_stack) {
          int space = max_stack - current_amount;
          int add = (p_amount < space) ? p_amount : space;

          slot["amount"] = current_amount + add;
          slots[i] = slot; // Write back
          p_amount -= add;

          if (p_amount <= 0) {
            emit_signal("inventory_changed");
            return true;
          }
        }
      }
    }
  }

  // 2. Try empty slots
  for (int i = 0; i < slots.size(); ++i) {
    Dictionary slot = slots[i];
    Ref<GameItem> slot_item = slot.get("item", Variant());

    if (slot_item.is_null()) {
      // Found empty
      slot["item"] = p_item;

      int max_stack = p_item->get_max_stack();
      int add = (p_amount < max_stack) ? p_amount : max_stack;

      slot["amount"] = add;
      slots[i] = slot;
      p_amount -= add;

      if (p_amount <= 0) {
        emit_signal("inventory_changed");
        return true;
      }
    }
  }

  // If we are here, we might have added some, but inventory is full.
  // For now, return false if not fully added, but we kept partial add.
  emit_signal("inventory_changed");
  return (p_amount <= 0);
}

bool Inventory::remove_item(const Ref<GameItem> &p_item, int p_amount) {
  if (p_item.is_null() || p_amount <= 0)
    return false;

  // Must find enough
  if (!has_item(p_item, p_amount))
    return false;

  for (int i = 0; i < slots.size(); ++i) {
    Dictionary slot = slots[i];
    Ref<GameItem> slot_item = slot.get("item", Variant());

    if (slot_item.is_valid() && slot_item == p_item) {
      int current_amount = slot["amount"];
      int remove = (p_amount < current_amount) ? p_amount : current_amount;

      slot["amount"] = current_amount - remove;
      if ((int)slot["amount"] <= 0) {
        slot["item"] = Variant();
        slot["amount"] = 0;
      }
      slots[i] = slot;

      p_amount -= remove;
      if (p_amount <= 0)
        break;
    }
  }

  emit_signal("inventory_changed");
  return true;
}

bool Inventory::has_item(const Ref<GameItem> &p_item, int p_amount) const {
  if (p_item.is_null())
    return false;

  int count = 0;
  for (int i = 0; i < slots.size(); ++i) {
    Dictionary slot = slots[i];
    Ref<GameItem> slot_item = slot.get("item", Variant());

    if (slot_item.is_valid() && slot_item == p_item) {
      count += (int)slot["amount"];
    }
  }
  return count >= p_amount;
}

void Inventory::clear() {
  for (int i = 0; i < slots.size(); ++i) {
    Dictionary slot;
    slot["item"] = Variant();
    slot["amount"] = 0;
    slots[i] = slot;
  }
  emit_signal("inventory_changed");
}
