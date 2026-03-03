#ifndef EVIDENCE_MANAGER_H
#define EVIDENCE_MANAGER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class EvidenceManager : public Node {
  GDCLASS(EvidenceManager, Node)

private:
  static EvidenceManager *singleton;

  // Stores the list of collected evidence/profile IDs
  Array collected_evidence;

protected:
  static void _bind_methods();

public:
  EvidenceManager();
  ~EvidenceManager();

  static EvidenceManager *get_singleton();

  bool add_evidence(const String &p_id);
  bool remove_evidence(const String &p_id);
  bool has_evidence(const String &p_id) const;
  Array get_all_evidence() const;
  void clear_all_evidence();

  Array serialize() const;
  void deserialize(const Array &p_data);
};

} // namespace godot

#endif // EVIDENCE_MANAGER_H
