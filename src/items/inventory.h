#ifndef INVENTORY_H
#define INVENTORY_H

#include "game_item.h"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;

// Simple slot structure
struct InventorySlot {
  Ref<GameItem> item;
  int amount;
};

class Inventory : public Resource {
  GDCLASS(Inventory, Resource)

private:
  // We need to Expose this as Array for Godot Inspector, but internally manage
  // struct? For simplicity in GDExtension Resource, let's use Array of
  // Dictionaries or just separate Arrays. Or better: Inventory is a Resource
  // that holds a list of Items. Let's implement a fixed size inventory for now.

  // Storing slots as Array of Dictionaries for easy serialization/inspection
  // default [ { "item": Res, "amount": 1 }, ... ]
  TypedArray<Dictionary> slots;
  int capacity;

protected:
  static void _bind_methods();

public:
  Inventory();
  ~Inventory();

  void set_capacity(int p_capacity);
  int get_capacity() const;

  void set_slots(const TypedArray<Dictionary> &p_slots);
  TypedArray<Dictionary> get_slots() const;

  // API
  bool add_item(const Ref<GameItem> &p_item, int p_amount = 1);
  bool remove_item(const Ref<GameItem> &p_item, int p_amount = 1);
  bool has_item(const Ref<GameItem> &p_item, int p_amount = 1) const;
  void clear();
};

#endif
