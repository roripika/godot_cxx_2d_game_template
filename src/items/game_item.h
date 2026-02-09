#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>

using namespace godot;

class GameItem : public Resource {
  GDCLASS(GameItem, Resource)

private:
  String item_name;
  String description;
  Ref<Texture2D> icon;
  bool stackable;
  int max_stack;

protected:
  static void _bind_methods();

public:
  GameItem();
  ~GameItem();

  void set_item_name(const String &p_name);
  String get_item_name() const;

  void set_description(const String &p_desc);
  String get_description() const;

  void set_icon(const Ref<Texture2D> &p_icon);
  Ref<Texture2D> get_icon() const;

  void set_stackable(bool p_stackable);
  bool is_stackable() const;

  void set_max_stack(int p_max);
  int get_max_stack() const;
};

#endif
