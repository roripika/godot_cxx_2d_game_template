#ifndef MYSTERY_MANAGER_H
#define MYSTERY_MANAGER_H

#include "enums.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <map>

using namespace godot;

namespace mystery {

class MysteryManager : public Node {
  GDCLASS(MysteryManager, Node)

private:
  static MysteryManager *singleton;
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
  MysteryManager();
  ~MysteryManager();

  static MysteryManager *get_singleton();

  // Flag Management (Type-safe version for C++)
  void set_mystery_flag(mystery::MysteryFlag p_flag, bool p_value);
  bool get_mystery_flag(mystery::MysteryFlag p_flag) const;

  // GDScript Bridge (String-based)
  void set_flag(const String &p_name, bool p_value);
  bool get_flag(const String &p_name) const;

  // State Management
  Dictionary serialize_state() const;
  void deserialize_state(const Dictionary &p_dict);
  void deserialize_flags(const Dictionary &p_dict); // Compatibility

  // Checkpoint System
  void save_checkpoint(const String &p_scene_path);
  String load_checkpoint();

  // Logging Helper
  void log_change(const String &p_type, const String &p_name,
                  const Variant &p_old, const Variant &p_new,
                  const String &p_caller);
  void validate_state();
};

} // namespace mystery

#endif // MYSTERY_MANAGER_H
