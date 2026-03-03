#ifndef MYSTERY_GAME_MASTER_H
#define MYSTERY_GAME_MASTER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <map>

namespace godot {

class MysteryGameMaster : public Node {
  GDCLASS(MysteryGameMaster, Node)

private:
  static MysteryGameMaster *singleton;
  std::map<String, bool> flags;

  // Checkpoint data
  bool has_checkpoint = false;
  String checkpoint_scene;
  Dictionary checkpoint_flags;
  Array checkpoint_evidence;
  int checkpoint_health = 3;

protected:
  static void _bind_methods();

public:
  MysteryGameMaster();
  ~MysteryGameMaster();

  static MysteryGameMaster *get_singleton();

  void set_flag(const String &p_name, bool p_value);
  bool get_flag(const String &p_name) const;

  Dictionary serialize_flags() const;
  void deserialize_flags(const Dictionary &p_dict);

  // Checkpoint System
  void save_checkpoint(const String &p_scene_path);
  String load_checkpoint();
};

} // namespace godot

#endif // MYSTERY_GAME_MASTER_H
