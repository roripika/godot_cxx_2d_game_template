#ifndef KARAKURI_TESTIMONY_SESSION_H
#define KARAKURI_TESTIMONY_SESSION_H

/**
 * @file karakuri_testimony_session.h
 * @brief Testimony confrontation session state.
 *
 * このファイルは `KarakuriScenarioRunner` から Testimony 進行ステートを
 * 切り出すことを示す設計文書として機能する。
 *
 * ## 現状
 * 以下のメンバーは現在 `KarakuriScenarioRunner` 内にフラットに保持されている:
 *   - `testimony_lines_`, `testimony_index_`, `testimony_round_`, `testimony_max_rounds_`
 *   - `testimony_active_`, `waiting_for_testimony_`, `waiting_for_evidence_selection_`
 *   - `pending_testimony_success_actions_`, `pending_testimony_failure_actions_`
 *
 * ## 切り出し方針（次期リファクタリング時）
 * 1. 以下の struct に全フィールドを移動する。
 * 2. Runner に `KarakuriTestimonySession testimony_{}` メンバーを追加する。
 * 3. Runner.cpp の `testimony_index_` 等を `testimony_.index` 等に置換する。
 *
 * ## 分離の理由
 * - testimony フェーズは Mystery 固有。他のゲームスタイル（Roguelike, Sandbox 等）
 *   が Runner を再利用する際にこのステートが不要な依存になる。
 * - Action 実行エンジン（`pending_actions_` 等）と証言セッションステートが
 *   混在していると将来の分割コストが高い。
 *
 * @see KarakuriScenarioRunner
 * @see docs/mystery_antigravity_handover.md §14.2
 */

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace karakuri {

/**
 * @brief 証言システム（Testimony confrontation）の進行ステートを保持する
 *        ストラクト。
 *
 * 現時点では KarakuriScenarioRunner の private セクションとして
 * フラットに保持される（本ファイルは将来の切り出しの準備）。
 * 次期リファクタリングでは Runner がこの struct型のメンバーを持つ形に移行する。
 *
 * @code
 * // 将来の Runner.h 使用イメージ:
 * KarakuriTestimonySession testimony_{};
 *
 * // 将来の Runner.cpp 使用イメージ:
 * testimony_.lines.clear();
 * testimony_.active = true;
 * @endcode
 */
struct KarakuriTestimonySession {
  /**
   * @brief 一行分の証言データ。
   *
   * 現在は KarakuriScenarioRunner::TestimonyLine として Runner 内に定義されている。
   * 切り出し時にここへ移動する。
   */
  struct Line {
    godot::String speaker_key;  ///< @brief 話者ローカライズキー
    godot::String speaker_text; ///< @brief 話者フォールバックテキスト
    godot::String text_key;     ///< @brief 台詞ローカライズキー
    godot::String text_text;    ///< @brief 台詞フォールバックテキスト
    godot::String evidence_id;  ///< @brief 正解の証拠ID（空なら矛盾なし）
    godot::String shake_key;    ///< @brief ゆさぶり応答ローカライズキー
    godot::String shake_text;   ///< @brief ゆさぶり応答フォールバックテキスト
    bool solved = false;        ///< @brief この行の矛盾が解決済みか
  };

  /** @brief 証言行のリスト（YAML `lines` リストから構築）。 */
  godot::Array lines;

  /** @brief 現在表示中の行インデックス（0-based）。 */
  int index = 0;

  /** @brief 最大ラウンド数（YAML `max_rounds`）。 */
  int max_rounds = 1;

  /** @brief 現在のラウンド番号（0-based）。 */
  int round = 0;

  /** @brief 現在 testimony フェーズが進行中か。 */
  bool active = false;

  /** @brief testimony の完了（成功/失敗）を待機中か。 */
  bool waiting = false;

  /** @brief 証拠選択 UI の応答待ちか（`present` アクション中）。 */
  bool waiting_for_evidence = false;

  /** @brief testimony 成功時に実行するアクション列。 */
  godot::Array success_actions;

  /** @brief testimony 失敗時（HP 切れ等）に実行するアクション列。 */
  godot::Array failure_actions;

  /** @brief セッション状態を初期値にリセットする。 */
  void reset() {
    lines.clear();
    index = 0;
    max_rounds = 1;
    round = 0;
    active = false;
    waiting = false;
    waiting_for_evidence = false;
    success_actions.clear();
    failure_actions.clear();
  }
};

} // namespace karakuri

#endif // KARAKURI_TESTIMONY_SESSION_H
