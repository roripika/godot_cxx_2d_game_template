#ifndef KARAKURI_SEQUENCE_PLAYER_H
#define KARAKURI_SEQUENCE_PLAYER_H

/**
 * @file sequence_player.h
 * @brief Basic Game Karakuri: タスクシーケンスを管理・実行するノード。
 *
 * ## 役割
 * - Ref<TaskBase> の配列（キュー）を順番に処理する。
 * - create_snapshot() で現在の状態を rollback_stack に保存する。
 * - rollback_to_last_snapshot() で直前のスナップショットに戻す。
 *   このとき ActionRunner::stop_all()・SoundService::stop_bgm() も呼ぶ。
 * - skip_all() で全タスクを complete_instantly() してシーケンスを即完了させる。
 *
 * ## NodePath 設定
 * インスペクタから scenario_runner_path を設定することで、
 * ロールバック時に ScenarioRunner の位置も復元する。
 *
 * ## シグナル
 * - sequence_finished: 全タスクが完了したとき
 * - rollback_performed: ロールバックが行われたとき
 */

#include "state_snapshot.h"
#include "task_base.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/node_path.hpp>

namespace karakuri {

class ScenarioRunner;
class ActionRunner;
class SoundService;

class SequencePlayer : public godot::Node {
  GDCLASS(SequencePlayer, godot::Node)

  godot::Array task_queue_;           ///< Ref<TaskBase> の配列
  int current_task_index_ = 0;
  bool running_ = false;
  bool started_current_ = false;      ///< 現在のタスクの on_start() を呼んだか

  godot::Array rollback_stack_;       ///< Ref<StateSnapshot> の配列

  godot::NodePath scenario_runner_path_;
  godot::NodePath action_runner_path_;

protected:
  static void _bind_methods();

public:
  SequencePlayer() = default;
  ~SequencePlayer() override = default;

  void _process(double delta) override;

  // ------------------------------------------------------------------
  // タスク管理
  // ------------------------------------------------------------------
  void add_task(const godot::Ref<TaskBase> &task);
  void clear_tasks();

  // ------------------------------------------------------------------
  // 再生制御
  // ------------------------------------------------------------------
  void start();
  void skip_all();
  bool is_running() const;

  // ------------------------------------------------------------------
  // スナップショット / ロールバック
  // ------------------------------------------------------------------
  void create_snapshot();
  void rollback_to_last_snapshot();

  // ------------------------------------------------------------------
  // NodePath プロパティ
  // ------------------------------------------------------------------
  void set_scenario_runner_path(const godot::NodePath &path);
  godot::NodePath get_scenario_runner_path() const;

  void set_action_runner_path(const godot::NodePath &path);
  godot::NodePath get_action_runner_path() const;

private:
  ScenarioRunner *find_scenario_runner() const;
  ActionRunner *find_action_runner() const;
  SoundService *find_sound_service() const;
  void advance();
  void finish_sequence();
};

} // namespace karakuri

#endif // KARAKURI_SEQUENCE_PLAYER_H
