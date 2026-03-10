#include "othello_manager.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace othello {

void OthelloManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("start_game"), &OthelloManager::start_game);
  ClassDB::bind_method(D_METHOD("is_valid_move", "x", "y", "color"),
                       &OthelloManager::is_valid_move);
  ClassDB::bind_method(D_METHOD("place_piece", "x", "y"),
                       &OthelloManager::place_piece);
  ClassDB::bind_method(D_METHOD("get_piece", "x", "y"),
                       &OthelloManager::get_piece);
  ClassDB::bind_method(D_METHOD("get_score", "color"),
                       &OthelloManager::get_score);
  ClassDB::bind_method(D_METHOD("get_current_turn"),
                       &OthelloManager::get_current_turn);
  ClassDB::bind_method(D_METHOD("set_current_turn", "p_turn"),
                       &OthelloManager::set_current_turn);

  ADD_PROPERTY(PropertyInfo(Variant::INT, "current_turn"), "set_current_turn",
               "get_current_turn");

  ADD_SIGNAL(MethodInfo("board_updated"));
  ADD_SIGNAL(
      MethodInfo("turn_changed", PropertyInfo(Variant::INT, "new_turn")));
  ADD_SIGNAL(MethodInfo("game_over", PropertyInfo(Variant::INT, "black_score"),
                        PropertyInfo(Variant::INT, "white_score")));
}

OthelloManager::OthelloManager() {
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      board[y][x] = 0;
    }
  }
}

OthelloManager::~OthelloManager() {}

void OthelloManager::start_game() {
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      board[y][x] = 0;
    }
  }

  // Initial setup: center 2x2
  board[3][3] = 2; // White
  board[3][4] = 1; // Black
  board[4][3] = 1; // Black
  board[4][4] = 2; // White

  current_turn_ = 1; // Black starts
  emit_signal("board_updated");
  emit_signal("turn_changed", current_turn_);
}

bool OthelloManager::is_valid_move(int x, int y, int color) const {
  if (x < 0 || x >= 8 || y < 0 || y >= 8 || board[y][x] != 0) {
    return false;
  }

  int opponent = (color == 1) ? 2 : 1;
  static const int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
  static const int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1};

  for (int dir = 0; dir < 8; ++dir) {
    int nx = x + dx[dir];
    int ny = y + dy[dir];
    bool found_opponent = false;

    while (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
      if (board[ny][nx] == opponent) {
        found_opponent = true;
      } else if (board[ny][nx] == color) {
        if (found_opponent)
          return true;
        else
          break;
      } else {
        break;
      }
      nx += dx[dir];
      ny += dy[dir];
    }
  }

  return false;
}

bool OthelloManager::place_piece(int x, int y) {
  if (!is_valid_move(x, y, current_turn_)) {
    return false;
  }

  board[y][x] = current_turn_;
  flip_pieces(x, y, current_turn_);
  emit_signal("board_updated");

  update_game_state();
  return true;
}

void OthelloManager::flip_pieces(int x, int y, int color) {
  int opponent = (color == 1) ? 2 : 1;
  static const int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
  static const int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1};

  for (int dir = 0; dir < 8; ++dir) {
    int nx = x + dx[dir];
    int ny = y + dy[dir];
    int count = 0;

    while (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
      if (board[ny][nx] == opponent) {
        count++;
      } else if (board[ny][nx] == color) {
        if (count > 0) {
          // Flip back to source
          int fx = x + dx[dir];
          int fy = y + dy[dir];
          for (int i = 0; i < count; ++i) {
            board[fy][fx] = color;
            fx += dx[dir];
            fy += dy[dir];
          }
        }
        break;
      } else {
        break;
      }
      nx += dx[dir];
      ny += dy[dir];
    }
  }
}

void OthelloManager::update_game_state() {
  int next_turn = (current_turn_ == 1) ? 2 : 1;

  if (can_place_anywhere(next_turn)) {
    current_turn_ = next_turn;
    emit_signal("turn_changed", current_turn_);
  } else if (can_place_anywhere(current_turn_)) {
    // Current player continues
    UtilityFunctions::print("Next player has no moves. Skipping turn.");
    emit_signal("turn_changed", current_turn_);
  } else {
    // Game Over
    emit_signal("game_over", get_score(1), get_score(2));
  }
}

bool OthelloManager::can_place_anywhere(int color) const {
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      if (is_valid_move(x, y, color))
        return true;
    }
  }
  return false;
}

int OthelloManager::get_piece(int x, int y) const {
  if (x < 0 || x >= 8 || y < 0 || y >= 8)
    return 0;
  return board[y][x];
}

int OthelloManager::get_score(int color) const {
  int score = 0;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      if (board[y][x] == color)
        score++;
    }
  }
  return score;
}

int OthelloManager::get_current_turn() const { return current_turn_; }
void OthelloManager::set_current_turn(int p_turn) { current_turn_ = p_turn; }

} // namespace othello
