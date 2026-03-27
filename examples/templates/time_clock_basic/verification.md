# Time/Clock Basic — 動作確認手順と実測値リファレンス

このファイルは Time/Clock テンプレートを使ったゲームの  
**runtime 検証（3 経路テスト）** の手順と期待値を定義します。  
特に **KernelClock の進行確認** と **timing window の境界値テスト** を中心に扱います。

---

## 1. 確認の方針

Time/Clock ゲームは以下の 3 経路すべてで動作が確認されている必要があります。

| 経路 | 終了条件 | `round:result` | `round:status` | シーン遷移先 |
| :--- | :--- | :--- | :--- | :--- |
| **clear** | 全ノーツ消化 かつ hit_count ≥ clear_hit_count | `"clear"` | `"cleared"` | `clear` シーン |
| **fail** | miss_count > max_miss_count（早期終了） または 全消化後 hit 不足 | `"fail"` | `"failed"` | `fail` シーン |
| **continue** | 全ノーツ未消化 かつ miss 許容内 | `""` | `"playing"` | `advance`（ループ） |

**Turn/Grid・Branching との差異**:
- シーンが `advance → judge → resolve → check_round` の 4 段サイクルを繰り返す
- miss_count が閾値を超えた時点で**即時 fail**（全ノーツ消化を待たない）

---

## 2. Smoke テスト手順

### 前提

- `samples/<your_game>/` に debug scene が配置されていること
- Debug Overlay が `clock:now_ms` / `judge:*` / `round:result` を表示できること

rhythm の例: `samples/rhythm_test/rhythm_debug_clear.tscn`

---

### ステップ 1: clear 経路を確認する

全ノーツに完全一致タップ（diff=0 → perfect）を設定して実行する。

```yaml
# clear 経路の boot シーン例
- action: setup_<your_game>_round
  payload:
    notes: [1000, 2000, 3000]   # ノーツ時刻
    taps:  [1000, 2000, 3000]   # tap 時刻 = note 時刻 → diff=0 → perfect
    advance_ms: 1000            # clock が 1000ms ずつ進む
    clear_hit_count: 3
    max_miss_count: 0
```

**確認すべき WorldState 値（rhythm_test 実測値 2026-03-24）**:

| キー | 期待値 |
| :--- | :--- |
| `chart:index` | 3（全ノーツ消化） |
| `chart:note_count` | 3 |
| `judge:perfect_count` | 3 |
| `judge:good_count` | 0 |
| `judge:miss_count` | 0 |
| `clock:now_ms` | 3000（advance_ms=1000 × 3 ステップ） |
| `round:result` | `"clear"` |
| `round:status` | `"cleared"` |

---

### ステップ 2: fail 経路を確認する（早期終了パターン）

全ノーツを未入力（tap=-1）にして max_miss を 1 に設定する。  
2 ミス発生時点で早期終了するため chart:index=2（3 番目未到達）になる。

```yaml
# fail 経路の boot シーン例
- action: setup_<your_game>_round
  payload:
    notes: [1000, 2000, 3000]
    taps:  [-1, -1, -1]         # 全未入力 → miss
    advance_ms: 1200            # clock が 1200ms ずつ進む（note_time + good_win を超える）
    clear_hit_count: 3
    max_miss_count: 1           # miss が 1 を超えたら即 fail
```

**確認すべき WorldState 値**:

| キー | 期待値 |
| :--- | :--- |
| `chart:index` | 2（3 番目のノーツ未到達で早期終了） |
| `judge:miss_count` | 2 |
| `clock:now_ms` | 2400（1200ms × 2 ステップ） |
| `round:result` | `"fail"` |
| `round:status` | `"failed"` |

**`advance_ms` と timing window の関係**:  
`advance_ms = 1200` のとき、1 ステップ後 `clock:now_ms = 1200`。  
`note_time = 1000`、`good_window_ms = 150` → `1000 + 150 = 1150 < 1200`  
→ `now_ms > note_time + good_window_ms` → miss 判定（tap=-1 でも成立）

---

### ステップ 3: continue 経路を確認する

1 ノーツのみ perfect、残 2 ノーツは未到達の状態で continue を観察する。

```yaml
# continue 経路の boot シーン例
- action: setup_<your_game>_round
  payload:
    notes: [1000, 2000, 3000]
    taps:  [1000, -1, -1]       # 1 番目のみ perfect
    advance_ms: 1000
    clear_hit_count: 3
    max_miss_count: 3           # ミスを許容（continue を維持）
```

**確認すべき WorldState 値**:

| キー | 期待値 |
| :--- | :--- |
| `chart:index` | 1（1 ノーツ処理後） |
| `judge:perfect_count` | 1 |
| `judge:miss_count` | 0 |
| `clock:now_ms` | 1000（1 ステップ後） |
| `round:result` | `""` |
| `round:status` | `"playing"` |
| シーン遷移 | `advance`（ループ継続） |

---

## 3. timing window 境界値テスト

timing window が正しく実装されているか確認するために、以下の境界値を試す。

### 3-a. perfect 境界（diff = perfect_window_ms = 50）

```yaml
notes: [1000]
taps:  [1050]   # diff = 50 → perfect の境界ぴったり
advance_ms: 1200  # now_ms = 1200 > 1000 → note時刻は過ぎている
# 期待: judge:last_result = "perfect"
```

### 3-b. good 境界（diff = good_window_ms = 150）

```yaml
notes: [1000]
taps:  [1150]   # diff = 150 → good の境界ぴったり
advance_ms: 1300
# 期待: judge:last_result = "good"
```

### 3-c. miss（diff > good_window_ms）

```yaml
notes: [1000]
taps:  [1200]   # diff = 200 > 150 → miss
advance_ms: 1300
# 期待: judge:last_result = "miss"
```

### 3-d. 未入力 miss（tap=-1, now > note + good_win）

```yaml
notes: [1000]
taps:  [-1]
advance_ms: 1200   # now_ms=1200 > 1000+150=1150 → miss
# 期待: judge:last_result = "miss"
```

### 3-e. 未入力 none（tap=-1, now ≤ note + good_win）

```yaml
notes: [1000]
taps:  [-1]
advance_ms: 900    # now_ms=900 < 1000+150=1150 → none（まだ判定時刻前）
# 期待: judge:last_result = "none"（proceed して次サイクルへ）
```

---

## 4. 確認チェックリスト

実装完了の判断基準として以下をすべて満たすこと。

- [ ] `clear` 経路が runtime で確認された（全ノーツ perfect）
- [ ] `fail` 経路が runtime で確認された（miss 早期終了）
- [ ] `continue` 経路が runtime で確認された（ループして advance に戻る）
- [ ] Debug Overlay で `clock:now_ms` が `advance_ms` ずつ増加することを確認
- [ ] timing window: perfect / good / miss 各境界値が正しく判定される
- [ ] `src/core/` のファイルに変更が加えられていない
- [ ] Validator が不正 payload（notes 昇順崩れ・advance_ms=0 等）でエラーを返す
- [ ] すべての non-boot シーンに pos-0 sacrifice が適用されている

---

## 5. 既知の落とし穴

### 5-1. pos-0 スキップバグ（Time/Clock は全 4 中間シーンで必要）

Turn/Grid・Branching より影響が大きいです。`advance / judge / resolve / check_round`  
の各シーンすべてで pos-0 に sacrifice を置かないと **クロックが半分しか進まない / 判定がスキップされる** 不具合が発生します。

```yaml
advance:
  on_enter:
    - action: advance_<your_game>_clock   # pos-0 ← dasmmy（スキップされる）
    - action: advance_<your_game>_clock   # pos-1 ← 実際に実行される
    - action: evaluate_<your_game>_round
      payload:
        if_clear: clear
        if_fail:  fail
        if_continue: judge
```

詳細: `docs/infrastructure_backlog.md`

### 5-2. advance_ms と note 時刻のズレ

`advance_ms = 1000`、`notes = [1000, 2000, 3000]` の場合、各ステップで  
clock が `1000 → 2000 → 3000` と進み note 時刻と一致します。  
`advance_ms = 1200` にすると `clock = 1200`、`note_time = 1000`、`diff = 200 > 150` → miss になります。  
設計時は `advance_ms ≈ note 間隔` を意識してください。

### 5-3. max_miss_count=0 と taps=-1 の組み合わせ

`max_miss_count=0` のとき、miss が 1 件発生した瞬間に `evaluate_rhythm_round` が  
`miss_count(1) > max_miss_count(0)` と判定して即 fail します。  
この挙動は仕様通りですが、continue テストには `max_miss_count >= 3` を使ってください。

### 5-4. FakeTap はノーツごとに 1 タップ

`load_fake_tap` は `chart:index` が指す番号のノーツに対応する tap 時刻を  
`tap:last_time_ms` に書き込みます。  
`resolve_rhythm_progress` が `chart:index` を +1 した後、次サイクルの `load_fake_tap` が  
次のノーツ用の時刻を書き込む実装になっています（FakeTap の `taps` 配列を参照）。

---

## 6. 参照実装の実測値（rhythm_test、2026-03-24 確認済み）

### clear 経路

```
notes = [1000, 2000, 3000], taps = [1000, 2000, 3000]
advance_ms = 1000, clear_hit_count = 3, max_miss_count = 0

chart:index        = 3
judge:perfect      = 3
judge:good         = 0
judge:miss         = 0
clock:now_ms       = 3000
round:result       = "clear"
round:status       = "cleared"
```

### fail 経路（早期終了）

```
notes = [1000, 2000, 3000], taps = [-1, -1, -1]
advance_ms = 1200, clear_hit_count = 3, max_miss_count = 1

chart:index        = 2   ← 3 番目のノーツ未到達で早期終了
judge:miss         = 2   ← 2 miss > max_miss=1 でトリガー
clock:now_ms       = 2400
round:result       = "fail"
round:status       = "failed"
```

### continue 経路

```
notes = [1000, 2000, 3000], taps = [1000, -1, -1]
advance_ms = 1000, clear_hit_count = 3, max_miss_count = 3

chart:index        = 1
judge:perfect      = 1
judge:miss         = 0
clock:now_ms       = 1000
round:result       = ""
round:status       = "playing"
→ advance シーンにループ継続
```

---

## 7. 自ゲームの実測値記録欄（ここを埋める）

ゲーム名: _______________  
確認日: _______________

| 経路 | 確認状態 | `chart:index` | `judge:perfect/good/miss` | `clock:now_ms` | `round:result` |
| :--- | :--- | :--- | :--- | :--- | :--- |
| clear | ⬜ 未確認 | | | | |
| fail | ⬜ 未確認 | | | | |
| continue | ⬜ 未確認 | | | | |

### timing window 境界値テスト記録欄

| テスト | tap 時刻 | diff(ms) | 期待判定 | 実測判定 | 確認 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| perfect 境界 | note+50 | 50 | perfect | | ⬜ |
| good 境界 | note+150 | 150 | good | | ⬜ |
| miss | note+200 | 200 | miss | | ⬜ |
| 未入力 miss | -1 (now>note+150) | — | miss | | ⬜ |
| 未入力 none | -1 (now≤note+150) | — | none | | ⬜ |
