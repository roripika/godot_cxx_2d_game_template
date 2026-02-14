#include "dialogue_ui.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void DialogueUI::_bind_methods() {
  ClassDB::bind_method(D_METHOD("show_message", "name", "text"),
                       &DialogueUI::show_message);
  ClassDB::bind_method(D_METHOD("hide_dialogue"), &DialogueUI::hide_dialogue);
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

  // Ensure this control fills the screen/canvas so children anchors work
  set_anchors_preset(PRESET_FULL_RECT);

  // Initialize UI components if not added via editor
  // Check if children exist
  background_panel = Object::cast_to<Panel>(get_node_or_null("Panel"));
  if (!background_panel) {
    background_panel = memnew(Panel);
    background_panel->set_name("Panel");
    // Anchor bottom
    background_panel->set_anchor(SIDE_LEFT, 0.1f);
    background_panel->set_anchor(SIDE_RIGHT, 0.9f);
    background_panel->set_anchor(SIDE_TOP, 0.7f);
    background_panel->set_anchor(SIDE_BOTTOM, 0.95f);
    add_child(background_panel);
  }

  name_label =
      Object::cast_to<Label>(background_panel->get_node_or_null("NameLabel"));
  if (!name_label) {
    name_label = memnew(Label);
    name_label->set_name("NameLabel");
    name_label->set_position(Vector2(10, -30)); // Above panel
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

  hide(); // Start hidden
}

void DialogueUI::_process(double delta) {
  if (is_typing) {
    current_display_index += type_speed * delta;
    if (current_display_index >= current_text.length()) {
      current_display_index = current_text.length();
      is_typing = false;
    }

    if (text_label) {
      text_label->set_text(current_text.substr(0, (int)current_display_index));
    }
  }
}

void DialogueUI::show_message(const String &name, const String &text) {
  current_text = text;
  current_display_index = 0.0f;
  is_typing = true;

  if (name_label)
    name_label->set_text(name);
  if (text_label)
    text_label->set_text("");

  show(); // Make visible
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
