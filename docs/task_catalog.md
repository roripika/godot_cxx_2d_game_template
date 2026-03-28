# Task Catalog

**Generated**: 2026-03-29  
**Source**: `src/games/*/tasks/*.cpp` (read-only scan)  
**Generator**: `tools/gen_task_catalog.py`  

> AI がシナリオを書く際の「どの Task を使うか」判断を支援するカタログ。  
> `src/core` には一切変更を加えていません。

## スコープ定義 (T7.1)

| 項目 | 値 |
|:---|:---|
| 走査対象 | `src/games/*/tasks/*.cpp`（`.h` は除外） |
| 除外領域 | `src/core/`（Kernel = 改変禁止領域） |
| カウント根拠 | billiards_test(4) + mystery_test(9) + rhythm_test(6) + roguelike_test(7) = **26** |
| 注意 | 開発ログ内に「30 tasks」との記述があるが、これは 2026-03-28 の初期ナレーションにおける計算ミス。正しい値は **26**。 |

---

## 概要

| モジュール | Task 数 | Waiting 使用 | テンプレート |
|:---|:---:|:---:|:---|
| [billiards_test](#billiards-test) | 4 | ✅ 1 件 | [link](../examples/templates/event_driven_basic/) |
| [mystery_test](#mystery-test) | 9 | ✅ 3 件 | [link](../examples/templates/branching_basic/) |
| [rhythm_test](#rhythm-test) | 6 | — | [link](../examples/templates/time_clock_basic/) |
| [roguelike_test](#roguelike-test) | 7 | — | [link](../examples/templates/turn_grid_basic/) |

---

## billiards_test — イベント駆動 (Event-Driven / Waiting)

参照テンプレート: [../examples/templates/event_driven_basic/](../examples/templates/event_driven_basic/)

| Task クラス | 責務 | 必須 payload key | 省略可能 payload key | WS 書き込みキー | WS 読み込みキー | Waiting |
|:---|:---|:---|:---|:---|:---|:---:|
| `EvaluateBilliardsRoundTask` | Evaluate Billiards Round | `if_clear`<br>`if_continue`<br>`if_fail` | — | `round:result` | `round:cue_ball_pocketed`<br>`round:shot_limit`<br>`round:shots_taken`<br>`round:target_1_pocketed`<br>`round:target_2_pocketed`<br>`round:target_count` |  |
| `RecordBilliardsEventTask` | Record Billiards Event | `event` | — | `event:last_name`<br>`round:cue_ball_pocketed`<br>`round:shots_taken`<br>`round:target_1_pocketed`<br>`round:target_2_pocketed` | `round:shots_taken`<br>`round:target_1_pocketed` |  |
| `SetupBilliardsRoundTask` | Setup Billiards Round | — | `shot_limit`<br>`target_count` | `event:last_name`<br>`round:cue_ball_pocketed`<br>`round:result`<br>`round:shot_limit`<br>`round:shots_taken`<br>`round:target_1_pocketed`<br>`round:target_2_pocketed`<br>`round:target_count` | — |  |
| `WaitForBilliardsEventTask` | Wait For Billiards Event | `events` | `timeout` | `event:last_name` | — | ✅ |

---

## mystery_test — 分岐推理 (Branching / Evidence)

参照テンプレート: [../examples/templates/branching_basic/](../examples/templates/branching_basic/)

| Task クラス | 責務 | 必須 payload key | 省略可能 payload key | WS 書き込みキー | WS 読み込みキー | Waiting |
|:---|:---|:---|:---|:---|:---|:---:|
| `AddEvidenceTask` | Add Evidence | `id` | — | `evidence:` | — |  |
| `CheckConditionTask` | Check Condition | `if_false`<br>`if_true` | `all_of`<br>`any_of` | — | `evidence:` |  |
| `CheckEvidenceTask` | Check Evidence | `id`<br>`if_false`<br>`if_true` | — | — | `evidence:` |  |
| `DiscoverEvidenceTask` | Discover Evidence | `location` | `evidence_id`<br>`id` | — | — |  |
| `EndGameTask` | End Game | `result` | — | — | — |  |
| `ParallelTaskGroup` | Parallel Task Group | `tasks` | — | — | — | ✅ |
| `SaveLoadTestTask` | Save Load Test | — | `key` | — | — |  |
| `ShowDialogueTask` | Show Dialogue | `speaker`<br>`text` | — | — | — | ✅ |
| `WaitForSignalTask` | Wait For Signal | `signal` | — | — | — | ✅ |

---

## rhythm_test — 時刻駆動 (Time/Clock)

参照テンプレート: [../examples/templates/time_clock_basic/](../examples/templates/time_clock_basic/)

| Task クラス | 責務 | 必須 payload key | 省略可能 payload key | WS 書き込みキー | WS 読み込みキー | Waiting |
|:---|:---|:---|:---|:---|:---|:---:|
| `AdvanceRhythmClockTask` | Advance Rhythm Clock | — | — | `clock:now_ms` | `config:advance_ms` |  |
| `EvaluateRhythmRoundTask` | Evaluate Rhythm Round | `if_clear`<br>`if_continue`<br>`if_fail` | — | `round:result`<br>`round:status` | `chart:index`<br>`chart:note_count`<br>`config:clear_hit_count`<br>`config:max_miss_count`<br>`judge:good_count`<br>`judge:miss_count`<br>`judge:perfect_count` |  |
| `JudgeRhythmNoteTask` | Judge Rhythm Note | — | — | `clock:now_ms`<br>`judge:last_result` | `chart:index`<br>`chart:note_count`<br>`config:good_window_ms`<br>`config:perfect_window_ms`<br>`tap:last_time_ms` |  |
| `LoadFakeTapTask` | Load Fake Tap | — | — | `clock:now_ms`<br>`tap:last_time_ms` | `chart:index`<br>`chart:note_count` |  |
| `ResolveRhythmProgressTask` | Resolve Rhythm Progress | — | — | `chart:index`<br>`judge:good_count`<br>`judge:miss_count`<br>`judge:perfect_count`<br>`tap:last_time_ms` | `chart:index`<br>`chart:note_count`<br>`judge:good_count`<br>`judge:last_result`<br>`judge:miss_count`<br>`judge:perfect_count` |  |
| `SetupRhythmRoundTask` | Setup Rhythm Round | `notes`<br>`taps` | `advance_ms`<br>`clear_hit_count`<br>`good_window_ms`<br>`max_miss_count`<br>`perfect_window_ms` | `chart:index`<br>`chart:note_count`<br>`clock:now_ms`<br>`config:advance_ms`<br>`config:clear_hit_count`<br>`config:good_window_ms`<br>`config:max_miss_count`<br>`config:perfect_window_ms`<br>`judge:good_count`<br>`judge:last_result`<br>`judge:miss_count`<br>`judge:perfect_count`<br>`round:result`<br>`round:status`<br>`tap:last_time_ms` | — |  |

---

## roguelike_test — ターン制グリッド (Turn/Grid)

参照テンプレート: [../examples/templates/turn_grid_basic/](../examples/templates/turn_grid_basic/)

| Task クラス | 責務 | 必須 payload key | 省略可能 payload key | WS 書き込みキー | WS 読み込みキー | Waiting |
|:---|:---|:---|:---|:---|:---|:---:|
| `ApplyEnemyTurnTask` | Apply Enemy Turn | — | — | `enemy_1:x`<br>`enemy_1:y`<br>`last_action:actor`<br>`last_action:type`<br>`player:hp`<br>`round:result` | `enemy_1:hp`<br>`enemy_1:x`<br>`enemy_1:y`<br>`player:hp`<br>`player:x`<br>`player:y` |  |
| `ApplyPlayerAttackTask` | Apply Player Attack | `target` | — | `enemy_1:hp`<br>`last_action:actor`<br>`last_action:type`<br>`round:result` | `enemy_1:hp`<br>`enemy_1:x`<br>`enemy_1:y`<br>`player:x`<br>`player:y` |  |
| `ApplyPlayerMoveTask` | Apply Player Move | — | — | `player:x`<br>`player:y`<br>`round:result` | `enemy_1:hp`<br>`enemy_1:x`<br>`enemy_1:y`<br>`goal:x`<br>`goal:y`<br>`last_action:type`<br>`player:x`<br>`player:y` |  |
| `EvaluateRoguelikeRoundTask` | Evaluate Roguelike Round | `if_clear`<br>`if_continue`<br>`if_fail` | — | — | `round:result` |  |
| `LoadFakePlayerCommandTask` | Load Fake Player Command | `command` | — | `last_action:actor`<br>`last_action:type`<br>`turn:phase` | — |  |
| `ResolveRoguelikeTurnTask` | Resolve Roguelike Turn | — | — | `last_action:actor`<br>`last_action:type`<br>`turn:index`<br>`turn:phase` | `turn:index` |  |
| `SetupRoguelikeRoundTask` | Setup Roguelike Round | — | `enemy_1_x`<br>`enemy_1_y`<br>`enemy_hp`<br>`player_hp`<br>`player_x`<br>`player_y` | `enemy_1:hp`<br>`enemy_1:x`<br>`enemy_1:y`<br>`goal:x`<br>`goal:y`<br>`last_action:actor`<br>`last_action:type`<br>`player:hp`<br>`player:x`<br>`player:y`<br>`round:result`<br>`turn:index`<br>`turn:phase` | — |  |

---

## 統計

- 総 Task 数: **26**
- モジュール数: **4**
- `TaskResult::Waiting` 使用 Task: **4**

_このファイルは自動生成です。手動編集しないでください。_  
_再生成: `python3 tools/gen_task_catalog.py`_
