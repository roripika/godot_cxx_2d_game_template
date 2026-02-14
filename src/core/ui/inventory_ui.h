#ifndef INVENTORY_UI_H
#define INVENTORY_UI_H

#include "../../items/inventory.h"
#include "inventory_slot_ui.h"
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/grid_container.hpp>

using namespace godot;

class InventoryUI : public Control {
  GDCLASS(InventoryUI, Control)

private:
  Ref<Inventory> inventory;
  GridContainer *grid_container = nullptr;

  // Settings
  int columns = 10;

protected:
  static void _bind_methods();
  void _notification(int p_what);

public:
  InventoryUI();
  ~InventoryUI();

  void set_inventory(const Ref<Inventory> &p_inventory);
  Ref<Inventory> get_inventory() const;

  void refresh();
  void _on_slot_clicked(int index, int button_index);

  void _ready() override;
};

#endif // INVENTORY_UI_H
