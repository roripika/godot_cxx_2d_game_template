# Rhythm Test Completion Memo

## 1. 概要
`rhythm_test` モジュールは、Kernel v2.0 の時間主導型（Time-driven）決定論の検証を目的に導入されました。スモークテストおよび静的監査の結果、設計通りの 3 経路（Clear / Fail / Continue）が正常に機能することを確認しました。

## 2. 検証結果 (Validation Results)

### A. Clear Path (`rhythm_clear_smoke.yaml`)
- **条件**: 3ノーツに対し、Perfect ウィンドウ内の時刻でフェイクタップを注入。
- **結果**: 
    - `judge:perfect_count` が 3 に到達。
    - `evaluate_rhythm_round` が `if_clear` を選択。
    - 最終的に `end_game { result: solved }` が実行される。

### B. Fail Path (`rhythm_fail_smoke.yaml`)
- **条件**: 全ノーツのタップ時刻を `-1` (タップなし) に設定。
- **結果**: 
    - `judge:miss_count` が `max_miss_count` を超過、または終了時に `clear_hit_count` 未達。
    - `evaluate_rhythm_round` が `if_fail` を選択。
    - 最終的に `end_game { result: failed }` が実行される。

### C. Continue Path (`rhythm_continue_smoke.yaml`)
- **条件**: 第1ノーツ消化後、第2ノーツ出現前にシナリオを終了。
- **結果**: 
    - 第1ノーツが Perfect 判定され、`chart:index` が 1 に進む。
    - `evaluate_rhythm_round` が `if_continue` を選択（`round:result` が空の状態を維持）。

## 3. 考察：Kernel v2.0 への貢献
- **時間精度の証明**: `KernelClock` が 1/60 秒単位の Task 更新ループと整合し、浮動小数点誤差に影響されない判定が成立することを確認しました。
- **決定論の維持**: フェイクタップ時刻を YAML で固定することで、リプレイ性の高いリズムテストが Core の変更なしで実現可能です。

## 4. 残課題 (Pending Items)
- **演出との同期**: 今回は論理判定のみを検証対象としたため、Godot の `AudioStreamPlayer` とのミリ秒単位でのハードウェア同期は次フェーズ以降の課題とします。
- **大量ノーツ負荷**: 100ノーツを超えるような長時間チャートにおける `WorldState` への負荷検証。
