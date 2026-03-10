#ifndef OTHELLO_MANAGER_H
#define OTHELLO_MANAGER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace othello {

class OthelloManager : public godot::Node {
  GDCLASS(OthelloManager, godot::Node)

private:
  int board[8][8];       // 0:Empty, 1:Black, 2:White
  int current_turn_ = 1; // 1:Black, 2:White

protected:
  static void _bind_methods();

public:
  OthelloManager();
  ~OthelloManager() override;

  void start_game();
  bool is_valid_move(int x, int y, int color) const;
  bool place_piece(int x, int y);
  int get_piece(int x, int y) const;
  int get_score(int color) const;

  int get_current_turn() const;
  void set_current_turn(int p_turn);

private:
  bool can_place_anywhere(int color) const;
  void update_game_state();
  void flip_pieces(int x, int y, int color);
};

} // namespace othello

#endif // OTHELLO_MANAGER_H
