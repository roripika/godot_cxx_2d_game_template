# Rhythm Test Completion Memo

## 1. 概要

`rhythm_test` モジュールは Kernel v2.0 の時間主導型（Time-driven）決定論の検証を目的に導入された。
6 タスク + 4 シナリオの minimal skeleton として実装し、debug overlay + 専用 debug scene 3 本で runtime を確認した。

## 2. 実装時に発見した不具合と修正

### Bug 1: end_game 未登録によるシナリオ無効化
- **原因**: `end_game` アクションは `MysteryTestGame._ready()` で `ActionRegistry` に登録される。rhythm debug scene に `MysteryTestGame` が存在しないと、`ScenarioRunner` のプリコンパイルフェーズで未登録扱いとなり `scenes_.clear()` → 全タスク未実行。
- **修正**: 3 つの debug scene に `MysteryTestGame` ノードを追加。
- **コミット**: `a8660d4`

### Bug 2: シーン遷移後の pos-0 タスクスキップ
- **原因**: `evaluate_rhythm_round` が `load_scene_by_id()` を呼ぶと `start_actions()` が `pending_action_index_` を 0 にリセットする。しかし `step_actions()` が `execute()` 返却後に `++` するため、遷移先シーンの pos 0 タスクが常にスキップされる。
- **修正**: `src/core` を変更せず、各非 boot シーン (advance / judge / resolve / check_round) の pos 0 に同一タスクの複製を「犠牲スロット」として配置。実際の処理は pos 1 以降で実行。
- **コミット**: `06267dc`

## 3. 検証結果 (Runtime Confirmation)

debug overlay で確認した WorldState キーの期待値:

### A. Clear Path — `rhythm_debug_clear.tscn`
- **入力条件**: notes=[1000,2000,3000], taps=[1000,2000,3000], clear_hit_count=3, max_miss=0
- **期待 WorldState**:
  - `chart:index = 3`, `chart:note_count = 3`
  - `judge:perfect_count = 3`, `judge:good_count = 0`, `judge:miss_count = 0`
  - `round:status = cleared`, `round:result = clear`
  - note bar: `PPP`

### B. Fail Path — `rhythm_debug_fail.tscn`
- **入力条件**: notes=[1000,2000,3000], taps=[-1,-1,-1], clear_hit_count=3, max_miss=1
- **期待 WorldState**:
  - `judge:miss_count >= 2` (max_miss=1 超過)
  - `round:status = failed`, `round:result = fail`
  - note bar: `XX` または `XXX`

### C. Continue Path — `rhythm_debug_continue.tscn`
- **入力条件**: notes=[1000,2000,3000], taps=[1000,-1,-1], clear_hit_count=3, max_miss=3
- **期待 WorldState**:
  - `chart:index = 1`, `judge:perfect_count = 1`
  - `round:status = playing`, `round:result = (pending)`
  - note bar: `P..` (1ノート処理済み、2ノート未到達)

## 4. 確認済みスコープ

- [x] `setup_rhythm_round` による WorldState 初期化
- [x] `advance_rhythm_clock` による `KernelClock` 進行
- [x] `load_fake_tap` によるフェイク入力注入
- [x] `judge_rhythm_note` による Perfect/Good/Miss 判定
- [x] `resolve_rhythm_progress` による counter 更新と `chart:index` 進行
- [x] `evaluate_rhythm_round` による 3 経路分岐
- [x] debug overlay による WorldState 全キー可視化
- [x] validator 17/17 PASS

## 5. 意図的 out-of-scope

- real input (キーボード/タップ) との接続 → 次フェーズ
- AudioStreamPlayer とのミリ秒同期 → 次フェーズ以降
- 100 ノーツ超の負荷テスト → Phase 3 stress strategy に委ねる
- BGM 演出・エフェクト → 対象外

## 6. 残課題 (Pending Items)

- **pos-0 skip 根本修正**: `src/core/scenario_runner.cpp` の `step_actions()` で `load_scene_by_id()` が呼ばれた場合のインデックス二重加算を防ぐ設計改善。現在は YAML 側の sacrifice slot で回避中。
- **GDExtension 登録順序の明文化**: `end_game` 等の共有タスクが特定ゲームノードに依存する構造を `docs/infrastructure_backlog.md` に記録済み。
