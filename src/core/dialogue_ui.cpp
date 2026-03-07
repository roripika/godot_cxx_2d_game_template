#include "dialogue_ui.h"
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace karakuri {

void DialogueUI::_bind_methods() {
  ClassDB::bind_method(D_METHOD("show_message", "name", "text"),
                       &DialogueUI::show_message);
  ClassDB::bind_method(D_METHOD("hide_dialogue"), &DialogueUI::hide_dialogue);
  ClassDB::bind_method(D_METHOD("_gui_input", "event"),
                       &DialogueUI::_gui_input);
  ADD_SIGNAL(MethodInfo("dialogue_finished"));
  ClassDB::bind_method(D_METHOD("skip_typing"), &DialogueUI::skip_typing);

  ClassDB::bind_method(D_METHOD("set_type_speed", "speed"),
                       &DialogueUI::set_type_speed);
  ClassDB::bind_method(D_METHOD("get_type_speed"), &DialogueUI::get_type_speed);
  ClassDB::bind_method(D_METHOD("is_typing_active"),
                       &DialogueUI::is_typing_active);

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "type_speed"), "set_type_speed",
               "get_type_speed");
}

DialogueUI::DialogueUI() {
  type_speed = 30.0f; // characters per second
  is_typing = false;
  current_display_index = 0.0f;
}

DialogueUI::~DialogueUI() {}

void DialogueUI::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  set_anchors_preset(PRESET_FULL_RECT);
  set_mouse_filter(MOUSE_FILTER_STOP);
  set_z_index(100);

  background_panel = Object::cast_to<Panel>(get_node_or_null("Panel"));
  if (!background_panel) {
    background_panel = memnew(Panel);
    background_panel->set_name("Panel");
    background_panel->set_anchor(SIDE_LEFT, 0.1f);
    background_panel->set_anchor(SIDE_RIGHT, 0.9f);
    background_panel->set_anchor(SIDE_TOP, 0.7f);
    background_panel->set_anchor(SIDE_BOTTOM, 0.95f);
    add_child(background_panel);
  }

  // Ensure solid background for visibility
  ColorRect *bg =
      Object::cast_to<ColorRect>(background_panel->get_node_or_null("SolidBG"));
  if (!bg) {
    bg = memnew(ColorRect);
    bg->set_name("SolidBG");
    bg->set_anchors_preset(PRESET_FULL_RECT);
    bg->set_color(Color(0.05, 0.05, 0.08, 0.9));
    background_panel->add_child(bg);
    background_panel->move_child(bg, 0);
  }

  name_label =
      Object::cast_to<Label>(background_panel->get_node_or_null("NameLabel"));
  if (!name_label) {
    name_label = memnew(Label);
    name_label->set_name("NameLabel");
    name_label->set_position(Vector2(10, -30));
    background_panel->add_child(name_label);
  }

  text_label = Object::cast_to<RichTextLabel>(
      background_panel->get_node_or_null("TextLabel"));
  if (!text_label) {
    text_label = memnew(RichTextLabel);
    text_label->set_name("TextLabel");
    text_label->set_anchor(SIDE_LEFT, 0.02f);
    text_label->set_anchor(SIDE_RIGHT, 0.98f);
    text_label->set_anchor(SIDE_TOP, 0.1f);
    text_label->set_anchor(SIDE_BOTTOM, 0.9f);
    background_panel->add_child(text_label);
  }

  hide();
}

void DialogueUI::_process(double delta) {
  if (is_typing) {
    current_display_index += type_speed * delta;
    if (current_display_index >= current_text.length()) {
      current_display_index = current_text.length();
      is_typing = false;
      // Removed auto-emit: wait for user input
    }

    if (text_label) {
      text_label->set_text(current_text.substr(0, (int)current_display_index));
    }
  }
}

void DialogueUI::_gui_input(const Ref<InputEvent> &p_event) {
  if (!is_visible())
    return;

  Ref<InputEventMouseButton> mb = p_event;
  bool is_confirm = (mb.is_valid() && mb->is_pressed() &&
                     mb->get_button_index() == MOUSE_BUTTON_LEFT);

  if (!is_confirm) {
    if (Input::get_singleton()->is_action_just_pressed("ui_accept") ||
        Input::get_singleton()->is_action_just_pressed("interact")) {
      is_confirm = true;
    }
  }

  if (is_confirm) {
    if (is_typing) {
      skip_typing();
    } else {
      godot::UtilityFunctions::print(
          "[DialogueUI] User confirmed message. Emitting dialogue_finished.");
      emit_signal("dialogue_finished");
    }
    get_viewport()->set_input_as_handled();
  }
}

void DialogueUI::show_message(const String &name, const String &text) {
  godot::UtilityFunctions::print(String("[DialogueUI] show_message: ") + name +
                                 " : " + text);
  current_text = text;
  current_display_index = 0.0f;
  is_typing = true;

  if (name_label)
    name_label->set_text(name);
  if (text_label)
    text_label->set_text("");

  show(); // Make visible

  // デバッグ用：確実に最前面に持ってくる
  set_z_index(100);
}

void DialogueUI::hide_dialogue() {
  hide();
  is_typing = false;
}

void DialogueUI::set_type_speed(float p_speed) { type_speed = p_speed; }

float DialogueUI::get_type_speed() const { return type_speed; }

bool DialogueUI::is_typing_active() const { return is_typing; }

void DialogueUI::skip_typing() {
  if (is_typing) {
    current_display_index = current_text.length();
    if (text_label)
      text_label->set_text(current_text);
    is_typing = false;
  }
}

} // namespace karakuri
