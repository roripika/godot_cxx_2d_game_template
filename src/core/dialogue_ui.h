#ifndef DIALOGUE_UI_H
#define DIALOGUE_UI_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>

using namespace godot;

class DialogueUI : public Control {
  GDCLASS(DialogueUI, Control)

private:
  Panel *background_panel;
  Label *name_label;
  RichTextLabel *text_label;

  // Internal state
  String current_text;
  float type_speed;
  float current_display_index;
  bool is_typing;

protected:
  static void _bind_methods();

public:
  DialogueUI();
  ~DialogueUI();

  void _ready() override;
  void _process(double delta) override;

  // API
  void show_message(const String &name, const String &text);
  void hide_dialogue();

  // Getters/Setters
  void set_type_speed(float p_speed);
  float get_type_speed() const;

  bool is_typing_active() const;
  void skip_typing();
};

#endif
