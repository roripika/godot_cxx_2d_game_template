# Event-Driven Basic — 動作確認手順

**参照実装**: `src/games/billiards_test/`  
**使用シナリオ**: `src/games/billiards_test/scenario/billiards_*.yaml`

---

## 0. 確認前の前提チェック

- [ ] GDExtension がビルド済み（`bin/` 以下に `.framework` が存在する）
- [ ] `billiards_test_game.cpp` の `_ready()` で 4 Task が登録されている  
      （`setup_billiards_round` / `wait_for_billiards_event` / `record_billiards_event` / `evaluate_billiards_round`）
- [ ] KernelClock が `_process` デルタで進む状態であること  
      （`WaitForBilliardsEventTask` は `KernelClock::now()` でタイムアウトを計測する）

---

## 1. clear 経路の確認（`billiards_fake_smoke.yaml`）

**シナリオ**: shot_committed → ball_pocketed → pocketed(1) >= target_count(1) → clear

### 手順

```bash
# Godot でシナリオを起動（Godot ログを出力させる）
/Applications/Godot.app/Contents/MacOS/Godot \
  --path /Users/ooharayukio/godot_cxx_2d_game_template \
  src/games/billiards_test/scenario/billiards_fake_smoke.yaml
```

### 期待ログ

```
[SetupBilliardsRoundTask] Round initialised: shot_limit=3 target_count=1
[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.
[RecordBilliardsEventTask] Recorded event: shot_committed
[RecordBilliardsEventTask] Recorded event: ball_pocketed
[EvaluateBilliardsRoundTask] Clear → victory
```

### 期待 WorldState（clear 完了時）

| キー | 期待値 | 確認 |
| :--- | :--- | :--- |
| `round:shots_taken` | 1 | ☐ |
| `round:target_1_pocketed` | true | ☐ |
| `round:target_2_pocketed` | false | ☐ |
| `round:cue_ball_pocketed` | false | ☐ |
| `round:result` | "clear" | ☐ |
| `event:last_name` | "ball_pocketed" | ☐ |

---

## 2. foul → fail 経路の確認（`billiards_foul_smoke.yaml`）

**シナリオ**: shot_committed → cue_ball_pocketed → ファウル優先 → fail

### 手順

```bash
/Applications/Godot.app/Contents/MacOS/Godot \
  --path /Users/ooharayukio/godot_cxx_2d_game_template \
  src/games/billiards_test/scenario/billiards_foul_smoke.yaml
```

### 期待ログ

```
[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.
[RecordBilliardsEventTask] Recorded event: shot_committed
[RecordBilliardsEventTask] Recorded event: cue_ball_pocketed
[EvaluateBilliardsRoundTask] Cue-ball foul → defeat
```

### 期待 WorldState（foul fail 完了時）

| キー | 期待値 | 確認 |
| :--- | :--- | :--- |
| `round:shots_taken` | 1 | ☐ |
| `round:cue_ball_pocketed` | true | ☐ |
| `round:result` | "fail" | ☐ |
| `event:last_name` | "cue_ball_pocketed" | ☐ |

### 注意: ファウル判定の優先順位確認

`evaluate_billiards_round` はまず `cue_ball_pocketed` を確認します。  
「ball_pocketed と cue_ball_pocketed が同時に起きてもファウル扱い」は、  
`record` の順序に関係なく ensure されているか確認してください。

---

## 3. continue → shot_limit fail 経路の確認（`billiards_continue_smoke.yaml`）

**シナリオ**: 3 ショット打つが 1 球もポケットできず、shot_limit 到達で fail

### 手順

```bash
/Applications/Godot.app/Contents/MacOS/Godot \
  --path /Users/ooharayukio/godot_cxx_2d_game_template \
  src/games/billiards_test/scenario/billiards_continue_smoke.yaml
```

### 期待ログ（3 ループ）

```
[EvaluateBilliardsRoundTask] Continue → shoot_again  ← 1 周目
[EvaluateBilliardsRoundTask] Continue → shoot_again  ← 2 周目
[EvaluateBilliardsRoundTask] Shot limit reached → defeat  ← 3 周目
```

### 期待 WorldState（shot_limit fail 完了時）

| キー | 期待値 | 確認 |
| :--- | :--- | :--- |
| `round:shots_taken` | 3 | ☐ |
| `round:target_1_pocketed` | false | ☐ |
| `round:cue_ball_pocketed` | false | ☐ |
| `round:result` | "fail" | ☐ |
| `event:last_name` | "balls_stopped" | ☐ |

---

## 4. TaskResult::Waiting メカニズムの確認

`WaitForBilliardsEventTask` が正しくポーリングしているかを確認します。

### テストケース

| No | 状況 | timeout 値 | 期待動作 |
| :--- | :--- | :--- | :--- |
| W-1 | smoke 正常 | 0.1 | タイムアウト後 balls_stopped 注入 → Success |
| W-2 | timeout=0 | 0.0 | ERR_INVALID_DATA（validate_and_setup で即エラー） |
| W-3 | events=[] | 空配列 | ERR_INVALID_DATA（validate_and_setup で即エラー） |
| W-4 | events=["unknown"] | 0.1 | ERR_INVALID_DATA（VALID_EVENTS にない名前） |

### W-1 の確認方法

`billiards_fake_smoke.yaml` を実行し、ログに  
`[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.`  
が出ることを確認する。

### W-2, W-3, W-4 の確認方法

以下のような不正 YAML を一時的に作成して実行し、ERR_INVALID_DATA が出ることを確認する:

```yaml
- action: wait_for_billiards_event
  payload:
    events: []         # ← W-3: 空配列
    timeout: 0.1
```

---

## 5. 確認チェックリスト

```
基本動作:
  [ ] clear 経路: 期待 WorldState が一致する
  [ ] foul → fail 経路: ファウル優先判定（cue_ball_pocketed 優先）が動作する
  [ ] continue → shot_limit fail: ループが 3 回まわり shots_taken=3 で止まる

Waiting メカニズム:
  [ ] wait_for_billiards_event が Waiting を返してフレーム待機している
  [ ] タイムアウト後に balls_stopped が event:last_name に書かれる
  [ ] timeout=0.0 で ERR_INVALID_DATA になる

Fail-Fast 検証:
  [ ] shot_limit=0 で setup_billiards_round が ERR_INVALID_DATA
  [ ] target_count=0 で setup_billiards_round が ERR_INVALID_DATA
  [ ] 不正イベント名で record_billiards_event が ERR_INVALID_DATA
  [ ] if_clear / if_fail / if_continue のどれかが省略されると evaluate が ERR_INVALID_DATA

WorldState スコープ:
  [ ] すべて SCOPE_SESSION に書かれている（GLOBAL への混入なし）
  [ ] 物理座標（x, y, velocity）が WorldState に書かれていない
```

---

## 6. 落とし穴と対処

### 落とし穴①：`shoot_again` で pos-0 が動かない

**症状**: continue ループで `shoot_again` に遷移したとき、最初のショットがスキップされる  
**原因**: シーン遷移後 pos-0 がスキップされる既知バグ  
**対処**: `shoot_again: on_enter:` の最初の Task を sacrifice コピーとして繰り返す

```yaml
shoot_again:
  on_enter:
    # ← pos-0: sacrifice（この行は実行されるが、無効化されても次の行で補完）
    - action: wait_for_billiards_event
      payload: { events: [balls_stopped], timeout: 0.1 }
    # ← pos-1 以降が実質的な処理
    - action: wait_for_billiards_event
      payload: { events: [balls_stopped], timeout: 0.1 }
    ...
```

### 落とし穴②：KernelClock が進まない → 永遠に Waiting

**症状**: シナリオが止まる（`end_game` に到達しない）  
**原因**: KernelClock の `advance()` が呼ばれていないため `now()` が更新されず、タイムアウトが永遠に来ない  
**対処**: KernelClock が Godot の `_process(delta)` と連動しているか確認する  
```
確認ポイント: KernelClock の実装で _process のデルタを自動 advance しているか
```

### 落とし穴③：foul とポケット同時は必ず fail になる

**症状**: ball_pocketed と cue_ball_pocketed を同一サイクルで record すると、  
「1 球入った」のに fail になる  
**原因**:这は仕様通り（evaluate はファウルを最優先判定する）  
**対処**: 仕様として受け入れる。foul 優先を変えたい場合は `evaluate` の判定順を変更する（contract.md 要更新）

### 落とし穴④：`event:last_name` は smoke での経路確認にのみ使う

**症状**: `event:last_name` を条件判定に使おうとするが、複数 record 後は最後の値のみ  
**原因**: `last_name` は最後のイベントのみ記録する（履歴ではない）  
**対処**: 判定は WorldState の boolean フラグ（`target_1_pocketed` 等）で行う。`last_name` はログ確認・デバッグ用

---

## 7. 実測値リファレンス（billiards_test、2026-03-28）

YAML シナリオと Task 実装から導出した期待値です（runtime 確認は次の実行時に記録してください）。

| 経路 | shots_taken | target_1_pocketed | cue_ball_pocketed | result | event:last_name |
| :--- | :--- | :--- | :--- | :--- | :--- |
| clear | 1 | true | false | "clear" | "ball_pocketed" |
| foul → fail | 1 | false | true | "fail" | "cue_ball_pocketed" |
| shot_limit → fail | 3 | false | false | "fail" | "balls_stopped" |

---

## 8. 自ゲームへの適用後に記録する欄

| 経路 | 計測日 | shots_taken | ポケットフラグ | result | event:last_name |
| :--- | :--- | :--- | :--- | :--- | :--- |
| clear | | | | | |
| foul → fail | | | | | |
| limit → fail | | | | | |
| continue N 周 | | | | | |
