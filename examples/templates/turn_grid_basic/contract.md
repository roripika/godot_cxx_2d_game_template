# Turn/Grid Basic — WorldState Contract & Task Contract

このファイルは Turn/Grid ゲームが Kernel と交わす**契約書**です。  
実装コードは含みません。新しいゲームを作る際にこの表を自分のゲーム用に埋めてください。

---

## 1. WorldState キー設計原則

| 原則 | 内容 |
| :--- | :--- |
| Namespace | `<your_game>:` プレフィックスを全キーに付ける |
| Scope | `SCOPE_SESSION`（1 セッション = 1 試合） |
| 型 | `int` または `String` のみ使用。浮動小数点は使わない |
| 分類 | 論理状態のみ。描画状態・フレーム毎の一時値は書かない |

---

## 2. WorldState キー仕様テーブル（roguelike_test 実装値）

新しいゲームを作る際はこの表を「雛形」として自分のキーに置き換えること。

### 2-a. ターン管理

| キー | 型 | スコープ | 初期値 | 書き込む Task |
| :--- | :--- | :--- | :--- | :--- |
| `<ns>:turn:index` | int | SESSION | 0 | `setup_round` / `resolve_turn` |
| `<ns>:turn:phase` | String | SESSION | `"player"` | `resolve_turn` |

### 2-b. プレイヤー

| キー | 型 | スコープ | 初期値 | 書き込む Task |
| :--- | :--- | :--- | :--- | :--- |
| `<ns>:player:x` | int | SESSION | payload | `setup_round` / `apply_player_move` |
| `<ns>:player:y` | int | SESSION | payload | `setup_round` / `apply_player_move` |
| `<ns>:player:hp` | int | SESSION | payload | `setup_round` / `apply_enemy_turn` |

### 2-c. エネミー

| キー | 型 | スコープ | 初期値 | 書き込む Task |
| :--- | :--- | :--- | :--- | :--- |
| `<ns>:enemy_1:x` | int | SESSION | default | `setup_round` / `apply_enemy_turn` |
| `<ns>:enemy_1:y` | int | SESSION | default | `setup_round` / `apply_enemy_turn` |
| `<ns>:enemy_1:hp` | int | SESSION | default | `setup_round` / `apply_player_attack` |

### 2-d. ゴール

| キー | 型 | スコープ | 初期値 | 書き込む Task |
| :--- | :--- | :--- | :--- | :--- |
| `<ns>:goal:x` | int | SESSION | default | `setup_round`（読み取り専用） |
| `<ns>:goal:y` | int | SESSION | default | `setup_round`（読み取り専用） |

### 2-e. ラウンド結果

| キー | 型 | スコープ | 初期値 | 書き込む Task |
| :--- | :--- | :--- | :--- | :--- |
| `<ns>:round:result` | String | SESSION | `""` | `evaluate_round` |
| `<ns>:last_action:actor` | String | SESSION | `""` | `resolve_turn` |
| `<ns>:last_action:type` | String | SESSION | `""` | `resolve_turn` |

---

## 3. Task 契約（責務・入力・出力・禁止事項）

### 3-1. setup_round

| 項目 | 内容 |
| :--- | :--- |
| **責務** | グリッド・エンティティ・HP をセッション初期化する |
| **payload** | `player_hp` / `enemy_hp` / `player_x` / `player_y`（すべて int、必須） |
| **読む WorldState** | なし |
| **書く WorldState** | player:x/y/hp, enemy_1:x/y/hp, goal:x/y, turn:index=0, turn:phase="player" |
| **Fail-Fast** | payload に欠損キーがあれば `validate_and_setup()` でエラーを返す |
| **禁止** | グリッドサイズ・ゴール位置などの定数をキーとして書かない（ハードコード可） |

### 3-2. load_fake_player_command

| 項目 | 内容 |
| :--- | :--- |
| **責務** | テスト用にプレイヤーコマンドを WorldState に書き込む（本番は実入力に置換） |
| **payload** | `command: <string>`（`move_up` / `move_down` / `move_left` / `move_right` / `attack`） |
| **読む WorldState** | なし |
| **書く WorldState** | `<ns>:player:command`（String） |
| **禁止** | 本番ビルドでは削除/置換すること。このタスクは smoke 専用 |

### 3-3. apply_player_move

| 項目 | 内容 |
| :--- | :--- |
| **責務** | `player:command` に従い `player:x / y` を 1 マス更新する |
| **payload** | なし（WorldState から読む） |
| **読む WorldState** | `player:command`, `player:x`, `player:y` |
| **書く WorldState** | `player:x`, `player:y` |
| **禁止** | グリッド境界チェックをスキップしない（壁抜け防止） |

### 3-4. apply_player_attack

| 項目 | 内容 |
| :--- | :--- |
| **責務** | プレイヤー隣接エネミーの HP を減らす |
| **payload** | なし |
| **読む WorldState** | `player:x/y`, `enemy_1:x/y`, `enemy_1:hp` |
| **書く WorldState** | `enemy_1:hp`（0 以上を保証） |
| **禁止** | 複数エネミーへの汎化はこのタスクの責務外（別タスクに分割） |

### 3-5. apply_enemy_turn

| 項目 | 内容 |
| :--- | :--- |
| **責務** | エネミーが 1 ステップ行動する（移動 or 攻撃） |
| **payload** | なし |
| **読む WorldState** | `enemy_1:x/y/hp`, `player:x/y/hp` |
| **書く WorldState** | `enemy_1:x/y`（移動時）、`player:hp`（攻撃時） |
| **禁止** | エネミーが dead（hp ≤ 0）の場合は行動しない |

### 3-6. resolve_roguelike_turn

| 項目 | 内容 |
| :--- | :--- |
| **責務** | 1 ターンの後処理（フェーズ更新、last_action 記録） |
| **payload** | なし |
| **読む WorldState** | `turn:index`, `player:x/y`, `enemy_1:x/y` |
| **書く WorldState** | `turn:index`（+1）, `turn:phase`, `last_action:actor`, `last_action:type` |
| **禁止** | 勝敗判定はここで行わない（evaluate_round の責務） |

### 3-7. evaluate_roguelike_round

| 項目 | 内容 |
| :--- | :--- |
| **責務** | 勝敗・継続を判定し `round:result` に書き込む。次シーンへ分岐を指示する |
| **payload** | `if_clear:` / `if_fail:` / `if_continue:`（すべてシーン名 String、必須） |
| **読む WorldState** | `player:x/y`, `goal:x/y`, `player:hp`, `enemy_1:hp` |
| **書く WorldState** | `round:result`（`"clear"` / `"fail"` / `""`） |
| **Fail-Fast** | 3 分岐 payload がすべて存在しなければエラー |

---

## 4. Task 追加時の契約拡張ルール

1. 新 Task のキー定義をこのファイルの 2 節に追記する。
2. 既存キーを**削除・リネーム**するときは、読み取り元 Task を全て更新する。
3. `round:result` への書き込みは `evaluate_round` 以外は行ってはいけない。
4. `src/core` のコードは変更しない（絶対制約）。

---

## 5. 禁止パターン

```
# ❌ WorldState に描画フラグを書く
world_state.set("player:is_blinking", 1)

# ❌ Task が直接シーン遷移する（ScenarioRunner に委ねるべき）
get_tree().change_scene("res://clear.tscn")

# ❌ 浮動小数点をグリッド座標に使う
world_state.set("player:x", 2.5)

# ❌ payload なしで setup_round を呼ぶ（Fail-Fast が必要）
- action: setup_roguelike_round
  # payload が空 → validate_and_setup() でエラーになること
```

---

## 6. 参照実装の WorldState 実測値（roguelike_test、2026-03-21 確認）

| パス | `player:x/y` → 終了時 | `player:hp` | `enemy_1:hp` | `round:result` | `turn:index` |
| :--- | :--- | :--- | :--- | :--- | :--- |
| clear | (4,3) → (4,4) | 3 | 2 | `"clear"` | 1 |
| fail | (1,1) → (1,1) | 0 | 2 | `"fail"` | 1+ |
| continue | 移動済み・未到達 | 1+ | 1+ | `""` | 1 |
