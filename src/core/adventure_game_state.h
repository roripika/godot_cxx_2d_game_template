#ifndef ADVENTURE_GAME_STATE_H
#define ADVENTURE_GAME_STATE_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

namespace karakuri {

class AdventureGameStateBase : public Node {
  GDCLASS(AdventureGameStateBase, Node)

private:
  static AdventureGameStateBase *singleton;
  int health = 3; // HP for adventure mode (default 3 for 3 strikes)
  godot::Callable reset_hook_; ///< @brief Mystery layer が登録するリセットフック

protected:
  static void _bind_methods();

public:
  AdventureGameStateBase();
  ~AdventureGameStateBase();

  static AdventureGameStateBase *get_singleton();

  // -------------------------------------------------------------------
  // HP 管理 (アドベンチャードメイン層のドメインロジック)
  // シーン遷移は SceneFlow (karakuri::SceneFlow) に責務分離済み。
  // -------------------------------------------------------------------
  void set_health(int hp);
  int get_health() const;
  void take_damage();
  void heal(int amount);

  // Game reset
  void reset_game();

  /**
   * @brief Mystery等の上位層がリセット時の追加処理を注入するためのフック。
   *        DI: mystery shell の _ready() から一度だけ登録する。
   */
  void set_reset_hook(const godot::Callable &hook);
};

} // namespace karakuri

#endif // ADVENTURE_GAME_STATE_H
