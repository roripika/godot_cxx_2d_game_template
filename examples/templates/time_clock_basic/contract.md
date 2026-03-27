# Time/Clock Basic — WorldState Contract & Task Contract

このファイルは Time/Clock ゲームが Kernel と交わす**契約書**です。  
実装コードは含みません。新しいゲームを作る際にこの表を自分のゲーム用に埋めてください。

---

## 1. WorldState キー設計原則

| 原則 | 内容 |
| :--- | :--- |
| Namespace | `<your_game>:` プレフィックスを全キーに付ける |
| Scope | すべて `SCOPE_SESSION`（1 セッション = 1 ラウンド） |
| 型 | `int`（ms），`String`（結果）のみ |
| 分類 | 論理状態のみ。音声バッファ・描画フラグは書かない |

---

## 2. WorldState キー仕様テーブル（rhythm_test 実装値）

### 2-a. チャート（ノーツ配置）

`setup_rhythm_round` がノーツ配列を個別キーに展開してSESSION に書き込む。

| キー | 型 | 書き込む Task | 内容 |
| :--- | :--- | :--- | :--- |
| `<ns>:chart:note_count` | int | `setup_rhythm_round` | 総ノーツ数（3〜5） |
| `<ns>:chart:note_<i>:time_ms` | int | `setup_rhythm_round` | ノーツ i の配置時刻（ms, i=0始まり） |
| `<ns>:chart:index` | int | `setup_rhythm_round` / `resolve_progress` | 現在処理中のノーツ インデックス（0始まり） |

### 2-b. 設定パラメータ

| キー | 型 | デフォルト | 書き込む Task | 内容 |
| :--- | :--- | :--- | :--- | :--- |
| `<ns>:config:advance_ms` | int | 1000 | `setup_rhythm_round` | 1 ステップで KernelClock を進める量（ms） |
| `<ns>:config:perfect_window_ms` | int | 50 | `setup_rhythm_round` | perfect 判定の許容誤差（ms） |
| `<ns>:config:good_window_ms` | int | 150 | `setup_rhythm_round` | good 判定の許容誤差（ms）、≥ perfect_window_ms |
| `<ns>:config:clear_hit_count` | int | note_count | `setup_rhythm_round` | クリアに必要な hit 数（perfect + good） |
| `<ns>:config:max_miss_count` | int | 1 | `setup_rhythm_round` | 許容ミス数（これを超えると即 fail） |

### 2-c. クロック

| キー | 型 | 書き込む Task | 内容 |
| :--- | :--- | :--- | :--- |
| `<ns>:clock:now_ms` | int | `advance_rhythm_clock` / `judge_rhythm_note` | KernelClock.now() × 1000 のスナップショット |

**重要**: `clock:now_ms` は WorldState の参照値であって、クロックの正体は  
`KernelClock::get_singleton()->now()` が返す double（秒）です。  
Task は必ず `KernelClock` を直接読み書きし、WorldState には結果だけを同期します。

### 2-d. タップ入力

| キー | 型 | 書き込む Task | 内容 |
| :--- | :--- | :--- | :--- |
| `<ns>:tap:last_time_ms` | int | `load_fake_tap` | 最後のタップ時刻（ms）。-1 = 未入力 |

### 2-e. 判定カウンタ

| キー | 型 | 初期値 | 書き込む Task | 内容 |
| :--- | :--- | :--- | :--- | :--- |
| `<ns>:judge:last_result` | String | `""` | `judge_rhythm_note` | 直前の判定結果（`"perfect"` / `"good"` / `"miss"` / `"none"`） |
| `<ns>:judge:perfect_count` | int | 0 | `resolve_rhythm_progress` | 累計 perfect 数 |
| `<ns>:judge:good_count` | int | 0 | `resolve_rhythm_progress` | 累計 good 数 |
| `<ns>:judge:miss_count` | int | 0 | `resolve_rhythm_progress` | 累計 miss 数 |

### 2-f. ラウンド結果

| キー | 型 | 書き込む Task | 内容 |
| :--- | :--- | :--- | :--- |
| `<ns>:round:result` | String | `evaluate_rhythm_round` | `"clear"` / `"fail"` / `""`（continuing） |
| `<ns>:round:status` | String | `evaluate_rhythm_round` | `"cleared"` / `"failed"` / `"playing"` |

---

## 3. KernelClock 利用ルール

```
KernelClock::advance(seconds)  ← advance_rhythm_clock だけが呼ぶ
KernelClock::now()             ← judge_rhythm_note だけが読む
```

| ルール | 内容 |
| :--- | :--- |
| advance は Task 以外から呼ばない | 直接 Godot ノードや GDScript から呼ぶと決定論が崩れる |
| now() の戻り値は double（秒）| WorldState への書き込みは `int(now() * 1000)` でミリ秒に変換する |
| advance_ms は YAML の payload で決める | ハードコードせず WorldState の `config:advance_ms` から読む |
| smoke シナリオは完全決定論的 | FakeTap (-1 / 実数) を YAML で固定するため毎回同じ結果になる |

---

## 4. Task 契約（責務・入力・出力・禁止事項）

### 4-1. setup_rhythm_round

| 項目 | 内容 |
| :--- | :--- |
| **責務** | チャート・設定パラメータ・カウンタを SESSION に初期化する |
| **payload** | `notes: [int, ...]`（必須, 3〜5要素, 昇順正数）/ `taps: [int, ...]`（必須, notes と同長, -1 以上）/ `advance_ms: int`（任意）/ `perfect_window_ms: int`（任意）/ `good_window_ms: int`（任意）/ `clear_hit_count: int`（任意）/ `max_miss_count: int`（任意） |
| **読む WorldState** | なし |
| **書く WorldState** | chart:note_count, chart:note_i:time_ms … , chart:index=0, all config:*, all judge:*=0, tap:last_time_ms=-1 |
| **Fail-Fast** | notes/taps 欠損・長さ不一致・降順・0以下・advance_ms<=0・good < perfect ウィンドウ でエラー |
| **禁止** | KernelClock を操作しない（初期化のみ WorldState を書く） |

### 4-2. advance_rhythm_clock

| 項目 | 内容 |
| :--- | :--- |
| **責務** | KernelClock を `config:advance_ms` ms だけ前進させ、`clock:now_ms` を更新する |
| **payload** | なし（WorldState から読む） |
| **読む WorldState** | `config:advance_ms` |
| **書く WorldState** | `clock:now_ms` |
| **KernelClock 操作** | `KernelClock::advance(advance_ms / 1000.0)` |
| **禁止** | advance_ms ≤ 0 のときはエラーを返す |

### 4-3. load_fake_tap

| 項目 | 内容 |
| :--- | :--- |
| **責務** | テスト用タップ時刻を WorldState に書き込む（本番は実入力に置換） |
| **payload** | `time_ms: int`（任意、デフォルト -1 = 未入力）/ YAML の `taps` リストから per-note で指定する実装も可 |
| **読む WorldState** | `chart:index`（何番目のノーツか） |
| **書く WorldState** | `tap:last_time_ms` |
| **禁止** | 本番ビルドでは削除/置換すること。このタスクは smoke 専用 |

### 4-4. judge_rhythm_note

| 項目 | 内容 |
| :--- | :--- |
| **責務** | `tap:last_time_ms` と `chart:note_i:time_ms` の差を比較して perfect/good/miss を判定する |
| **payload** | なし |
| **読む WorldState** | `chart:index`, `chart:note_i:time_ms`, `tap:last_time_ms`, `config:perfect_window_ms`, `config:good_window_ms` |
| **書く WorldState** | `judge:last_result`, `clock:now_ms` |
| **KernelClock 操作** | `KernelClock::now()` を読む（書かない） |
| **判定ロジック** | `diff = \|tap_time - note_time\|` → diff ≤ perfect_win → "perfect"、diff ≤ good_win → "good"、else → "miss"。tap=-1 かつ `now_ms > note_time + good_win` → "miss" |
| **禁止** | カウンタ（perfect_count 等）を直接書かない（resolve_progress の責務） |

### 4-5. resolve_rhythm_progress

| 項目 | 内容 |
| :--- | :--- |
| **責務** | `judge:last_result` をカウンタに反映し、`chart:index` を +1 する |
| **payload** | なし |
| **読む WorldState** | `judge:last_result`, `judge:perfect_count`, `judge:good_count`, `judge:miss_count`, `chart:index`, `chart:note_count` |
| **書く WorldState** | 該当カウンタ +1、`chart:index` +1、`tap:last_time_ms = -1` |
| **禁止** | `judge:last_result == "none"` のときは何も更新しない |

### 4-6. evaluate_rhythm_round

| 項目 | 内容 |
| :--- | :--- |
| **責務** | カウンタと index を見て clear/fail/continue を判定し次シーンへ分岐する |
| **payload** | `if_clear: <scene_id>`（必須）/ `if_fail: <scene_id>`（必須）/ `if_continue: <scene_id>`（必須） |
| **読む WorldState** | `chart:index`, `chart:note_count`, `judge:perfect_count`, `judge:good_count`, `judge:miss_count`, `config:clear_hit_count`, `config:max_miss_count` |
| **書く WorldState** | `round:result`, `round:status` |
| **判定ロジック** | `miss_count > max_miss_count` → fail（即時）。index ≥ note_count → hit≥clear_hit → clear、else → fail。それ以外 → continue |
| **Fail-Fast** | 3 分岐 payload がすべて存在しなければエラー |

---

## 5. timing window 設計早見表

| notes 値 | taps 値 | diff(ms) | 判定 | 備考 |
| :--- | :--- | :--- | :--- | :--- |
| 1000 | 1000 | 0 | perfect（≤50ms） | 完全一致 |
| 1000 | 1050 | 50 | perfect（≤50ms） | 境界ギリギリ |
| 1000 | 1100 | 100 | good（≤150ms） | |
| 1000 | 1200 | 200 | miss（>150ms） | |
| 1000 | -1 | — | miss（未入力） | now_ms > 1000+150 の場合 |

---

## 6. 禁止パターン

```yaml
# ❌ judge_rhythm_note の前に advance_rhythm_clock を忘れる
#    → clock:now_ms がノーツ時刻に届かないため miss になる
- action: load_fake_tap
  payload:
    time_ms: 1000
- action: judge_rhythm_note    # clock:now_ms = 0 → diff の代わりに miss 判定

# ❌ advance_ms と notes の時刻を合わせないと全て miss になる
#    notes: [1000, 2000, 3000], advance_ms: 500
#    → clock は 500ms → first note(1000ms) に到達しない

# ❌ evaluate_rhythm_round を呼ばずにシーン遷移する
#    → round:result が書かれないためデバッグ不能

# ❌ WorldState に音声バッファのデータを書く
world_state.set("beat:audio_samples", audio_data)  # ❌
```

---

## 7. 参照実装の WorldState 実測値（rhythm_test、2026-03-24 確認済み）

| 経路 | `chart:index` | `judge:perfect/good/miss` | `clock:now_ms` | `round:result` |
| :--- | :--- | :--- | :--- | :--- |
| clear | 3 | 3 / 0 / 0 | 3000 | `"clear"` |
| fail | 2 | 0 / 0 / 2 | 2400 | `"fail"` |
| continue | 1 | 1 / 0 / 0 | 1000 | `""` |

**fail 経路は index=2 で早期終了**:  
miss=2 > max_miss=1 → 即 fail（3 番目のノーツ未到達でも終了）
