#include "inventory_slot_ui.h"
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

InventorySlotUI::InventorySlotUI() {
  set_custom_minimum_size(Vector2(40, 40)); // Default size
}

InventorySlotUI::~InventorySlotUI() {}

void InventorySlotUI::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_slot_index", "index"),
                       &InventorySlotUI::set_slot_index);
  ClassDB::bind_method(D_METHOD("get_slot_index"),
                       &InventorySlotUI::get_slot_index);

  ADD_SIGNAL(MethodInfo("slot_clicked", PropertyInfo(Variant::INT, "index"),
                        PropertyInfo(Variant::INT, "button_index")));
}

void InventorySlotUI::_ready() {
  // Create child nodes if they don't exist (if instantiated from script only)
  // But usually we might use a scene for layout.
  // For Universal approach, let's building children procedurally ensures it
  // works without extensive scene setup, OR we rely on scene injection. Let's
  // build procedurally for simplicity self-containedness.

  if (!icon_texture) {
    icon_texture = memnew(TextureRect);
    icon_texture->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
    icon_texture->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
    icon_texture->set_anchors_preset(Control::PRESET_FULL_RECT);
    add_child(icon_texture);
  }

  if (!amount_label) {
    amount_label = memnew(Label);
    amount_label->set_anchors_preset(Control::PRESET_BOTTOM_RIGHT);
    amount_label->set_text("");
    add_child(amount_label);
  }
}

void InventorySlotUI::set_slot_data(const Ref<GameItem> &p_item, int p_amount) {
  item = p_item;
  amount = p_amount;

  if (icon_texture) {
    if (item.is_valid()) {
      icon_texture->set_texture(item->get_icon()); // Assuming get_icon exists
    } else {
      icon_texture->set_texture(Ref<Texture2D>());
    }
  }

  if (amount_label) {
    if (amount > 1) {
      amount_label->set_text(String::num(amount));
    } else {
      amount_label->set_text("");
    }
  }
}

void InventorySlotUI::set_slot_index(int p_index) { slot_index = p_index; }
int InventorySlotUI::get_slot_index() const { return slot_index; }

void InventorySlotUI::_notification(int p_what) {
  // Normal Control behavior
}

void InventorySlotUI::_gui_input(const Ref<InputEvent> &p_event) {
  Ref<InputEventMouseButton> mb = p_event;
  if (mb.is_valid() && mb->is_pressed()) {
    emit_signal("slot_clicked", slot_index, mb->get_button_index());
  }
}
