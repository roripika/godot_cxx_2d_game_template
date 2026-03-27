# Event-Driven Basic — WorldState Contract & Task Contract

**参照実装**: `src/games/billiards_test/`

---

## 1. WorldState キー設計原則

- **スコープ**: すべて `SCOPE_SESSION`（セッション内限定）
- **名前空間**: `<your_game>` に統一（参照実装は `billiards_test`）
- **座標・速度は書かない**: 物理演算の結果（x/y/velocity 等）は WorldState に書いてはならない。書くのは「何が起きたか（イベント名）」と「その累積（カウント・フラグ）」のみ

---

## 2. WorldState キー仕様テーブル（billiards_test 実装値）

| キー | 型 | 初期値 | 書き込む Task | 読み取る Task |
| :--- | :--- | :--- | :--- | :--- |
| `round:shots_taken` | int | 0 | `record_billiards_event("shot_committed")` | `evaluate_billiards_round` |
| `round:shot_limit` | int | payload | `setup_billiards_round` | `evaluate_billiards_round` |
| `round:target_count` | int | payload | `setup_billiards_round` | `evaluate_billiards_round` |
| `round:target_1_pocketed` | bool | false | `record_billiards_event("ball_pocketed")` | `evaluate_billiards_round` |
| `round:target_2_pocketed` | bool | false | `record_billiards_event("ball_pocketed")` | `evaluate_billiards_round` |
| `round:cue_ball_pocketed` | bool | false | `record_billiards_event("cue_ball_pocketed")` | `evaluate_billiards_round` |
| `round:result` | str | "" | `evaluate_billiards_round` | — |
| `event:last_name` | str | "" | `wait_for_billiards_event`（タイムアウト時），`record_billiards_event` | — |

### WorldState に書いてはいけないもの

```
❌ ボールの座標（x, y, z）
❌ 速度（velocity, speed）
❌ 角度（angle, direction）
❌ 物理演算の内部状態
→ これらはすべて BilliardsManager（物理エンジン側）が管理する
```

---

## 3. 有効イベント名（VALID_EVENTS）

`WaitForBilliardsEventTask` と `RecordBilliardsEventTask` が共通で検証する列挙値:

| イベント名 | 意味 | `record` 時の WorldState 更新 |
| :--- | :--- | :--- |
| `shot_committed` | ショット確定 | `round:shots_taken` +1 |
| `ball_pocketed` | 的球ポケット | `target_1_pocketed` → `target_2_pocketed` を順に true |
| `cue_ball_pocketed` | 手球ポケット（ファウル） | `round:cue_ball_pocketed` = true |
| `balls_stopped` | 全球停止（サイクル区切り） | 変更なし（イベント名のみ記録） |

新しいイベントを追加するには `WaitForBilliardsEventTask::VALID_EVENTS[]` と  
`RecordBilliardsEventTask::VALID[]` の **両方**を同時に更新すること。

---

## 4. Task 契約（責務・入力・出力・禁止事項）

### 4-1. setup_billiards_round

| 項目 | 内容 |
| :--- | :--- |
| **責務** | WorldState をラウンド初期状態にリセットする |
| **payload** | `shot_limit: int`（> 0）、`target_count: int`（> 0） |
| **読む WorldState** | なし |
| **書く WorldState** | `round:shots_taken=0`、`round:shot_limit`、`round:target_count`、`round:target_*_pocketed=false`、`round:cue_ball_pocketed=false`、`round:result=""`、`event:last_name=""` |
| **Fail-Fast** | `shot_limit <= 0` または `target_count <= 0` の場合 ERR_INVALID_DATA |
| **禁止** | 物理演算の呼び出し・座標の書き込み |

---

### 4-2. wait_for_billiards_event ← **このテンプレートの核心 Task**

| 項目 | 内容 |
| :--- | :--- |
| **責務** | 指定した物理イベントが届くまで待機し、タイムアウト時は `balls_stopped` を自動注入する |
| **payload** | `events: [event_name, ...]`（VALID_EVENTS から選択）、`timeout: float`（> 0。smoke 用: 0.1、本番: 30.0 程度） |
| **読む WorldState** | なし（KernelClock::now() のみ参照） |
| **書く WorldState** | タイムアウト時: `event:last_name = "balls_stopped"` |
| **戻り値** | タイムアウト未達: `TaskResult::Waiting`／タイムアウト達: `TaskResult::Success` |
| **Fail-Fast** | `events` が空・VALID_EVENTS 外の名前・`timeout <= 0` の場合 ERR_INVALID_DATA |
| **禁止** | `TaskResult::Success` をタイムアウト前に返すこと（物理シグナル受信は `complete_instantly()` 経由） |

#### Waiting パターンの詳細

```cpp
// validate_and_setup:
start_time_ = KernelClock::get_singleton()->now();

// execute() — 毎フレーム呼ばれる:
double now = KernelClock::get_singleton()->now();
if (now - start_time_ >= timeout_) {
    ws->set_state(..., "event:last_name", "balls_stopped");
    return TaskResult::Success;
}
return TaskResult::Waiting;
```

#### 本番との切り替え

```
smoke テスト:  timeout: 0.1  （すぐタイムアウト → balls_stopped 注入）
本番ゲーム:    timeout: 30.0 （物理エンジンが complete_instantly() を呼ぶまで待機）
```

---

### 4-3. record_billiards_event

| 項目 | 内容 |
| :--- | :--- |
| **責務** | イベント名を受け取り、対応する WorldState フラグをインクリメント・セットする |
| **payload** | `event: string`（VALID_EVENTS の中から 1 つ） |
| **読む WorldState** | `round:target_1_pocketed`（ball_pocketed の振り分けに使用） |
| **書く WorldState** | イベント種別に応じて round: キー群と `event:last_name` を更新 |
| **Fail-Fast** | VALID_EVENTS 外の event 名: ERR_INVALID_DATA |
| **禁止** | 物理演算ロジックの記述・複数キーの条件分岐（is clear? は evaluate に委ねる） |

---

### 4-4. evaluate_billiards_round

| 項目 | 内容 |
| :--- | :--- |
| **責務** | WorldState に基づいて clear / fail / continue を判定し、シーンを遷移させる |
| **payload** | `if_clear: scene_id`、`if_fail: scene_id`、`if_continue: scene_id`（3 つすべて必須） |
| **読む WorldState** | `round:cue_ball_pocketed`、`round:target_*_pocketed`、`round:target_count`、`round:shots_taken`、`round:shot_limit` |
| **書く WorldState** | `round:result`（"clear" / "fail" / "active"） |
| **判定優先順位** | ①ファウル優先（`cue_ball_pocketed=true` → fail）②クリア③ショット上限 fail ④continue |
| **Fail-Fast** | `if_clear`/`if_fail`/`if_continue` のうち 1 つでも欠けた場合 ERR_INVALID_DATA |
| **禁止** | 4 パターン（cue_foul / pocketed / shot_limit / continue）以外の独自判定を追加すること |

---

## 5. Task 追加時の契約拡張ルール

1. 新しいイベント種別を追加する場合は `VALID_EVENTS[]` 配列（wait / record 両方）を同時更新する。
2. 新しい WorldState キーを追加する場合は本ファイルの 2 節のテーブルに追記する。
3. 物理演算の座標・速度を WorldState に書く Task は追加してはならない。
4. `evaluate_billiards_round` の判定ロジックを Task 以外（YAML / setup 等）で実装してはならない。

---

## 6. 禁止パターン

```yaml
# ❌ 物理座標を WorldState に書く
- action: record_ball_position
  payload:
    x: 1.5       # → WorldState に書かない
    y: 2.3       # → 物理エンジン側の責務

# ❌ wait の前に record/evaluate を置く
- action: record_billiards_event    # wait なし！ balls_stopped か確認できていない
  payload: { event: ball_pocketed }

# ❌ evaluate を複数シーンに分散させる
# → 分岐判定は evaluate_billiards_round 1 点に集約すること

# ❌ wait_for_billiards_event を Waiting なしで実装する（即 Success を返す）
# → smoke の場合でも timeout 経由で Success を返すこと（直接 Success は Fake-event 契約違反）
```

---

## 7. 参照実装の WorldState 実測値（billiards_test、2026-03-28 確認）

| 経路 | shots_taken | target_1_pocketed | cue_ball_pocketed | result | event:last_name |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **clear** | 1 | true | false | "clear" | "ball_pocketed" |
| **foul → fail** | 1 | false | true | "fail" | "cue_ball_pocketed" |
| **shot_limit → fail** | 3 | false | false | "fail" | "balls_stopped" |
