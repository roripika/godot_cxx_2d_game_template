#ifndef INVENTORY_SLOT_UI_H
#define INVENTORY_SLOT_UI_H

#include "../../items/inventory.h"
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/texture_rect.hpp>

using namespace godot;

class InventorySlotUI : public Control {
  GDCLASS(InventorySlotUI, Control)

private:
  Ref<GameItem> item;
  int amount = 0;
  int slot_index = -1;

  TextureRect *icon_texture = nullptr;
  Label *amount_label = nullptr;

protected:
  static void _bind_methods();

public:
  void _notification(int p_what);
  void _gui_input(const Ref<InputEvent> &p_event) override;

  InventorySlotUI();
  ~InventorySlotUI();

  void set_slot_data(const Ref<GameItem> &p_item, int p_amount);
  void set_slot_index(int p_index);
  int get_slot_index() const;

  void _ready() override;
};

#endif // INVENTORY_SLOT_UI_H
