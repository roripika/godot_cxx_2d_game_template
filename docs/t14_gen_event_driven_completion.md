# T14 完了メモ — Event-Driven Basic Generator

**完了日**: 2026-04-01  
**Generator**: `tools/gen_scenario_event_driven.py`  
**template**: `event_driven_basic`  
**ベースモジュール**: `src/games/billiards_test/`

---

## 1. R-8 / R-9 / R-10 headless smoke 確認結果

### R-8: TaskResult::Waiting headless 動作

**smoke YAML**: `src/games/billiards_test/scenario/billiards_r8_waiting_smoke.yaml`  
**debug tscn**: `samples/billiards_test/billiards_debug_r8_waiting.tscn`

```
[SetupBilliardsRoundTask] Round initialised: shot_limit=3 target_count=1
[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.    ← R-8 確認
[RecordBilliardsEventTask] Recorded event: ball_pocketed
[EvaluateBilliardsRoundTask] Clear → victory
[ScenarioRunner] load_scene_by_id: victory
[ScenarioRunner] starting compiled actions for victory. Action count: 2.0
[EndGameTask] Game ended with result: solved                      ← R-10 確認
```

**結論**:
- R-8 ✅: `_process(delta)` → `clock->advance(delta)` が正しく呼ばれ、`timeout: 0.1` 後に `balls_stopped` が注入される
- R-10 ✅: terminal に pos-0 sacrifice を適用した場合、`victory` Action count: 2.0 → pos-0 skip → pos-1 `end_game(solved)` 実行

---

### R-9: continue シーン pos-0 sacrifice 動作

**smoke YAML**: `src/games/billiards_test/scenario/billiards_r9_continue_smoke.yaml`  
**debug tscn**: `samples/billiards_test/billiards_debug_r9_continue.tscn`

```
[ScenarioRunner] load_scene_by_id: setup_round
[ScenarioRunner] starting compiled actions for setup_round. Action count: 4.0
[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.      ← setup_round の wait
[EvaluateBilliardsRoundTask] Continue → shoot_again

[ScenarioRunner] load_scene_by_id: shoot_again
[ScenarioRunner] starting compiled actions for shoot_again. Action count: 4.0
                                   ← pos-0 (wait sacrifice) スキップ
[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.      ← pos-1 の actual wait
[EvaluateBilliardsRoundTask] Clear → victory

[ScenarioRunner] load_scene_by_id: victory
[ScenarioRunner] starting compiled actions for victory. Action count: 2.0
[EndGameTask] Game ended with result: solved
```

**結論**:
- R-9 ✅: `shoot_again` Action count: 4.0（wait×2 + record + evaluate）
  - pos-0: `wait_for_billiards_event` sacrifice → スキップ
  - pos-1: `wait_for_billiards_event` actual → `TaskResult::Waiting` ポーリング → timeout → `balls_stopped` 注入 → `Success`
  - pos-2: `record_billiards_event(ball_pocketed)` 実行
  - pos-3: `evaluate_billiards_round` → Clear → victory

---

## 2. Generator 出力仕様

### 生成シーン構成

| シーン | pos-0 sacrifice | 内容 |
|:---|:---:|:---|
| `setup_round` (boot) | 不要 | `setup_billiards_round` + `wait` + `records` + `evaluate` |
| `<scenes.continue>` | **必要** | `wait×2` (sacrifice+actual) + `records` + `evaluate` |
| `<terminal_clear>` | **必要** | `end_game×2` (sacrifice+actual) |
| `<terminal_fail>` | **必要** | `end_game×2` (sacrifice+actual) |

### WorldState キー（evaluate 判定用）

| キー | 設定タスク |
|:---|:---|
| `round:shots_taken` | `record_billiards_event("shot_committed")` で +1 |
| `round:shot_limit` | `setup_billiards_round.shot_limit` |
| `round:target_count` | `setup_billiards_round.target_count` |
| `round:target_1_pocketed` | `record_billiards_event("ball_pocketed")` 1球目 |
| `round:target_2_pocketed` | `record_billiards_event("ball_pocketed")` 2球目 |
| `round:cue_ball_pocketed` | `record_billiards_event("cue_ball_pocketed")` |
| `event:last_name` | `wait_for_billiards_event` timeout / `record_billiards_event` |

### evaluate 判定優先順位

```
1. cue_ball_pocketed == true              → if_fail  (ファウル優先)
2. pocketed >= target_count              → if_clear
3. shots_taken >= shot_limit             → if_fail   (ショット上限)
4. otherwise                             → if_continue
```

---

## 3. smoke テスト結果

### エラー系（V-ED）

| テスト | 期待 | 結果 |
|:---|:---:|:---:|
| V-ED-01: template = billiards_basic | exit 1 | ✅ |
| V-ED-03: shot_limit = 99 | exit 1 | ✅ |
| V-ED-09: terminal_clear = terminal_fail | exit 1 | ✅ |
| V-ED-10: terminal_clear = setup_round（予約語） | exit 1 | ✅ |

### 正常系

| テスト | 期待 | 結果 |
|:---|:---:|:---:|
| 最小構成（billiards_clear） | generate + validate exit 0 | ✅ |
| 推奨構成（billiards_challenge, 全オプション） | generate + validate exit 0 | ✅ |
| 期待出力サンプル保存 → validate | validate exit 0 | ✅ |

---

## 4. 既知の制約（v1.0 スコープ外）

- **target_count = 2 の complete smoke 未実施**: generator は `target_count=2` をサポートしているが、2球 pocketed のフルフロー headless テストは未作成
- **foul（cue_ball_pocketed）経路の generator smoke 未実施**: `boot_records: [cue_ball_pocketed]` → fail の動作確認は手動確認のみ
- **複数 wait_events のリアルイベント実装は Kernel 外**: `WaitForBilliardsEventTask` の実 physics 連携（Kernel 外）は billiards_test のスコープ外
- **continue_records の shot_limit 超過経路**: continue ループが shot_limit に達するケースの headless テストは未作成

---

## 5. billiards_test_completion.md の「未確認」事項への対応

`billiards_test_completion.md` セクション 9「pos-0 sacrifice の影響未確認」は本 T14 smoke で解消済み:

- 既存の `billiards_fake_smoke.yaml` / `billiards_continue_smoke.yaml` / `billiards_foul_smoke.yaml` は terminal に pos-0 sacrifice がなく、`end_game` が実行されていない（バグ）
- T14 Generator は全ての terminal / continue シーンに pos-0 sacrifice を自動配置することで問題を解消している
- 既存 yaml の修正は T14 のスコープ外（破壊的変更になるため）
