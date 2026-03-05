#ifndef KARAKURI_TESTIMONY_SESSION_H
#define KARAKURI_TESTIMONY_SESSION_H

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace mystery {

/**
 * @brief 証言システム（Testimony confrontation）の進行ステートを保持する
 *        ストラクト。
 */
struct TestimonySession {
  struct Line {
    godot::String speaker_key;
    godot::String speaker_text;
    godot::String text_key;
    godot::String text_text;
    godot::String evidence_id;
    godot::String shake_key;
    godot::String shake_text;
    bool solved = false;
  };

  godot::Array lines;
  int index = 0;
  int max_rounds = 1;
  int round = 0;
  bool active = false;
  bool waiting = false;
  bool waiting_for_evidence = false;
  godot::Array success_actions;
  godot::Array failure_actions;

  void reset(const godot::String & /*reason*/) {
    active = false;
    waiting = false;
    waiting_for_evidence = false;
    index = 0;
    round = 0;
    max_rounds = 1;
    lines.clear();
    success_actions.clear();
    failure_actions.clear();
  }
};

} // namespace mystery

#endif // KARAKURI_TESTIMONY_SESSION_H
