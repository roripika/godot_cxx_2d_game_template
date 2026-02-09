#include "game_item.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GameItem::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_item_name", "name"),
                       &GameItem::set_item_name);
  ClassDB::bind_method(D_METHOD("get_item_name"), &GameItem::get_item_name);
  ClassDB::bind_method(D_METHOD("set_description", "description"),
                       &GameItem::set_description);
  ClassDB::bind_method(D_METHOD("get_description"), &GameItem::get_description);
  ClassDB::bind_method(D_METHOD("set_icon", "icon"), &GameItem::set_icon);
  ClassDB::bind_method(D_METHOD("get_icon"), &GameItem::get_icon);
  ClassDB::bind_method(D_METHOD("set_stackable", "stackable"),
                       &GameItem::set_stackable);
  ClassDB::bind_method(D_METHOD("is_stackable"), &GameItem::is_stackable);
  ClassDB::bind_method(D_METHOD("set_max_stack", "max_stack"),
                       &GameItem::set_max_stack);
  ClassDB::bind_method(D_METHOD("get_max_stack"), &GameItem::get_max_stack);

  ADD_PROPERTY(PropertyInfo(Variant::STRING, "item_name"), "set_item_name",
               "get_item_name");
  ADD_PROPERTY(PropertyInfo(Variant::STRING, "description",
                            PROPERTY_HINT_MULTILINE_TEXT),
               "set_description", "get_description");
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "icon",
                            PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"),
               "set_icon", "get_icon");
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stackable"), "set_stackable",
               "is_stackable");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "max_stack"), "set_max_stack",
               "get_max_stack");
}

GameItem::GameItem() {
  item_name = "New Item";
  description = "";
  stackable = true;
  max_stack = 99;
}

GameItem::~GameItem() {}

void GameItem::set_item_name(const String &p_name) { item_name = p_name; }

String GameItem::get_item_name() const { return item_name; }

void GameItem::set_description(const String &p_desc) { description = p_desc; }

String GameItem::get_description() const { return description; }

void GameItem::set_icon(const Ref<Texture2D> &p_icon) { icon = p_icon; }

Ref<Texture2D> GameItem::get_icon() const { return icon; }

void GameItem::set_stackable(bool p_stackable) { stackable = p_stackable; }

bool GameItem::is_stackable() const { return stackable; }

void GameItem::set_max_stack(int p_max) { max_stack = p_max; }

int GameItem::get_max_stack() const { return max_stack; }
