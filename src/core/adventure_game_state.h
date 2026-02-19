#ifndef ADVENTURE_GAME_STATE_H
#define ADVENTURE_GAME_STATE_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

class AdventureGameStateBase : public Node {
  GDCLASS(AdventureGameStateBase, Node)

private:
  static AdventureGameStateBase *singleton;
  Dictionary flags;
  Array inventory; // Storing String names of items
  int health = 3;  // HP for adventure mode (default 3 for 3 strikes)

protected:
  static void _bind_methods();

public:
  AdventureGameStateBase();
  ~AdventureGameStateBase();

  static AdventureGameStateBase *get_singleton();

  void set_flag(const String &key, bool value);
  bool get_flag(const String &key, bool default_value = false) const;

  void add_item(const String &item_name);
  void remove_item(const String &item_name);
  bool has_item(const String &item_name) const;

  void change_scene(const String &path);
  
  // HP management
  void set_health(int hp);
  int get_health() const;
  void take_damage();
  void heal(int amount);
  
  // Game reset
  void reset_game();

  // --- Snapshot API (used by KarakuriSaveService) ---
  /** @brief flags 全体のコピーを返す (Dictionary<String, bool>)。 */
  godot::Dictionary get_flags_snapshot() const;
  /** @brief flags をスナップショットから一括復元して上書きする。 */
  void restore_flags_snapshot(const godot::Dictionary &snapshot);
  /** @brief inventory 全体のコピーを返す (Array<String>)。 */
  godot::Array get_inventory_snapshot() const;
  /** @brief inventory をスナップショットから一括復元して上書きする。 */
  void restore_inventory_snapshot(const godot::Array &snapshot);
};

#endif // ADVENTURE_GAME_STATE_H
