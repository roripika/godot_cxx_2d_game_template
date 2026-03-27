# billiards_test 完了メモ

**モジュール**: `src/games/billiards_test/`  
**カテゴリ**: Event-Driven / TaskResult::Waiting / Fake-event タイムアウト  
**確認日**: 2026-03-28  
**テンプレート**: `examples/templates/event_driven_basic/`（Phase 3-A T6）

---

## 1. モジュール概要

`billiards_test` は、Jolt Physics による物理演算（ボール軌道・衝突）を  
**Kernel 側のゲームロジックから完全に切り離す**設計を検証するモジュールです。

- `BilliardsManager`（`src/billiards/`）が物理シミュレーションを担当
- `billiards_test`（`src/games/billiards_test/`）が Kernel のゲームロジックを担当
- 両者は **WorldState のイベント文字列**のみで繋がり、座標・速度は WorldState に書かない

この「物理演算とゲームロジックの境界」が billiards_test の最大の設計要点です。

---

## 2. Task 一覧

| Task クラス | 登録名 | 責務 |
| :--- | :--- | :--- |
| `SetupBilliardsRoundTask` | `setup_billiards_round` | WorldState を初期化（shot_limit, target_count 等） |
| `WaitForBilliardsEventTask` | `wait_for_billiards_event` | **TaskResult::Waiting** でポーリング。タイムアウトで `balls_stopped` を注入 |
| `RecordBilliardsEventTask` | `record_billiards_event` | イベント名を受け取り、対応する WorldState フラグを更新 |
| `EvaluateBilliardsRoundTask` | `evaluate_billiards_round` | clear / fail / continue を判定してシーン遷移 |

---

## 3. WorldState キー仕様

すべて `SCOPE_SESSION`、名前空間 `billiards_test`。

| キー | 型 | 初期値 | 書き込む Task |
| :--- | :--- | :--- | :--- |
| `round:shots_taken` | int | 0 | `record_billiards_event("shot_committed")` で +1 |
| `round:shot_limit` | int | payload | `setup_billiards_round` |
| `round:target_count` | int | payload | `setup_billiards_round` |
| `round:target_1_pocketed` | bool | false | `record_billiards_event("ball_pocketed")` 1球目 |
| `round:target_2_pocketed` | bool | false | `record_billiards_event("ball_pocketed")` 2球目 |
| `round:cue_ball_pocketed` | bool | false | `record_billiards_event("cue_ball_pocketed")` |
| `round:result` | str | "" | `evaluate_billiards_round`（"clear"/"fail"/"active"） |
| `event:last_name` | str | "" | `wait_for_billiards_event`（タイムアウト時），`record_billiards_event` |

---

## 4. 有効イベント名（`VALID_EVENTS`）

`WaitForBilliardsEventTask` と `RecordBilliardsEventTask` が共に検証する文字列リスト:

| イベント名 | 意味 | WorldState への影響 |
| :--- | :--- | :--- |
| `shot_committed` | ショット確定 | `round:shots_taken` +1 |
| `ball_pocketed` | 的球ポケット | `target_1_pocketed` / `target_2_pocketed` を順に true |
| `cue_ball_pocketed` | 手球ポケット（ファウル） | `round:cue_ball_pocketed` = true |
| `balls_stopped` | 全球停止 | WorldState 変更なし（サイクル区切りのイベント） |

---

## 5. evaluate_billiards_round の判定ロジック

```
判定優先順位（上から評価）:
  1. cue_ball_pocketed == true               → if_fail（ファウル優先）
  2. pocketed >= target_count                → if_clear
  3. shots_taken >= shot_limit               → if_fail（ショット上限）
  4. それ以外                                → if_continue
```

`pocketed = (target_1_pocketed ? 1 : 0) + (target_2_pocketed ? 1 : 0)`

---

## 6. TaskResult::Waiting のメカニズム

`WaitForBilliardsEventTask` は Kernel テストで固有の **Waiting ポーリング** を実装します。

```
validate_and_setup():
  start_time_ = KernelClock::now()

execute() が毎フレーム呼ばれる:
  now = KernelClock::now()
  if now - start_time_ >= timeout_:
      event:last_name = "balls_stopped"  ← Fake-event 注入
      return TaskResult::Success          ← 次の Task へ
  else:
      return TaskResult::Waiting          ← フレーム待機
```

**smoke テスト用配慮**: `timeout: 0.1`（100ms）を指定することで、  
実際の物理演算を待たず即タイムアウトさせ、決定論的シナリオを実現する。

---

## 7. シナリオ構成と経路

### 7-1. clear 経路（`billiards_fake_smoke.yaml`）

```
setup_round
  └─ setup_billiards_round (shot_limit=3, target_count=1)
  └─ wait_for_billiards_event (timeout=0.1) → balls_stopped 注入
  └─ record_billiards_event (shot_committed) → shots_taken=1
  └─ record_billiards_event (ball_pocketed) → target_1_pocketed=true
  └─ evaluate_billiards_round → pocketed(1) >= target_count(1) → victory

victory → end_game(result: solved)
```

**期待 WorldState（clear 完了時）:**
| キー | 期待値 |
| :--- | :--- |
| `round:shots_taken` | 1 |
| `round:target_1_pocketed` | true |
| `round:target_2_pocketed` | false |
| `round:cue_ball_pocketed` | false |
| `round:result` | "clear" |
| `event:last_name` | "ball_pocketed" |

---

### 7-2. foul → fail 経路（`billiards_foul_smoke.yaml`）

```
setup_round
  └─ setup_billiards_round (shot_limit=3, target_count=1)
  └─ wait_for_billiards_event (timeout=0.1) → balls_stopped 注入
  └─ record_billiards_event (shot_committed) → shots_taken=1
  └─ record_billiards_event (cue_ball_pocketed) → cue_ball_pocketed=true
  └─ evaluate_billiards_round → cue_foul=true → defeat

defeat → end_game(result: failed)
```

**期待 WorldState（foul fail 完了時）:**
| キー | 期待値 |
| :--- | :--- |
| `round:shots_taken` | 1 |
| `round:cue_ball_pocketed` | true |
| `round:result` | "fail" |
| `event:last_name` | "cue_ball_pocketed" |

---

### 7-3. continue → shot_limit fail 経路（`billiards_continue_smoke.yaml`）

```
setup_round
  └─ setup_billiards_round (shot_limit=3, target_count=1)
  └─ wait → shot_committed (shots=1) → evaluate → continue → shoot_again
shoot_again (2周目)
  └─ wait → shot_committed (shots=2) → evaluate → continue → shoot_again
shoot_again (3周目)
  └─ wait → shot_committed (shots=3) → evaluate → shots(3) >= limit(3) → defeat

defeat → end_game(result: failed)
```

**期待 WorldState（shot_limit fail 完了時）:**
| キー | 期待値 |
| :--- | :--- |
| `round:shots_taken` | 3 |
| `round:target_1_pocketed` | false |
| `round:cue_ball_pocketed` | false |
| `round:result` | "fail" |
| `event:last_name` | "balls_stopped" |

---

## 8. 他モジュールとの根本的な差異

| 観点 | others (turn / branching / rhythm) | billiards_test |
| :--- | :--- | :--- |
| TaskResult の種類 | Success / Failed のみ | **Success / Failed / Waiting** |
| 入力モック方式 | FakeCommand / FakeTap（即時） | **Fake-event タイムアウト**（時間経過で自動注入） |
| KernelClock 利用 | advance（rhythm のみ）/ now | **now のみ**（タイムアウト計測専用） |
| 物理座標の WorldState | なし | なし（BilliardsManager が担当。座標はイベント通知のみ） |
| ループの起点 | boot or setup（1シーン） | `shoot_again`（`if_continue` で自己参照ループ） |
| pos-0 sacrifice | 必要 | YAML 内では未確認（要確認） |

---

## 9. 既知の制約・注意事項

- **BilliardsManager の物理ループ（`_physics_process`）は Kernel の Task ループとは独立している**  
  → 実際のゲームでは `WaitForBilliardsEventTask` の `execute()` が物理エンジンからの  
  シグナルを受けて早期リターンする仕組みが必要だが、現テストでは timeout で代替

- **`target_2_pocketed` は smoke シナリオで未使用**  
  → `target_count=2` の経路テストが未作成

- **pos-0 sacrifice の影響未確認**  
  → `shoot_again` シーン（`if_continue` 遷移先）で pos-0 のスキップが発生する可能性がある。  
  `billiards_continue_smoke.yaml` の結果が 2 周ではなく 1 周で終わる場合はここを疑う

- **`timeout` 値はフレームレートに依存する**  
  → `timeout: 0.1` は KernelClock が Godot の `_process` と同期している前提。  
  KernelClock が `advance` されない場合は永遠に Waiting 状態になる
