# Turn/Grid Basic — 動作確認手順と実測値リファレンス

このファイルは Turn/Grid テンプレートを使ったゲームの  
**runtime 検証（3 経路テスト）** の手順と期待値を定義します。

---

## 1. 確認の方針

Turn/Grid ゲームは以下の 3 経路すべてで動作が確認されている必要があります。

| 経路 | 終了条件 | `round:result` | シーン遷移先 |
| :--- | :--- | :--- | :--- |
| **clear** | プレイヤーがゴールマスに到達 | `"clear"` | `clear` シーン |
| **fail** | プレイヤー HP が 0 以下 | `"fail"` | `fail` シーン |
| **continue** | ゴール未到達・HP 残存 | `""` | `boot` に戻る（ループ） |

---

## 2. Smoke テスト手順

### 前提

- Godot エディタまたは `--headless` CLI で対象シーンを起動できること
- Debug Overlay が WorldState を読み取れること

### ステップ 1: debug シーンを用意する

```
samples/<your_game>/
└── <your_game>_debug_scene.tscn  ← debug overlay + GameNode の配置
```

roguelike の例: `samples/roguelike/roguelike_debug_scene.tscn`

### ステップ 2: clear 経路を確認する

YAML の初期配置を「1 手でゴール到達」に設定して実行する。

```yaml
# clear 経路の boot シーン例
- action: setup_<your_game>_round
  payload:
    player_hp: 3
    enemy_hp:  2
    player_x:  4
    player_y:  3   # ゴール (4,4) の 1 マス手前

- action: load_fake_player_command
  payload:
    command: move_down
```

**確認すべき WorldState 値（roguelike_test 実測値）**:

| キー | 期待値 |
| :--- | :--- |
| `round:result` | `"clear"` |
| `player:x` | 4 |
| `player:y` | 4 |
| `player:hp` | 3（被ダメなし） |
| `enemy_1:hp` | 2（攻撃なし） |
| `turn:index` | 1 |

### ステップ 3: fail 経路を確認する

プレイヤー HP が 0 になる初期配置で実行する。

```yaml
# fail 経路の boot シーン例（プレイヤーが敵に隣接、HP が削られる配置）
- action: setup_<your_game>_round
  payload:
    player_hp: 1    # ← HP を低く設定
    enemy_hp:  3    # ← 敵 HP を高く設定（倒されない）
    player_x:  2
    player_y:  2    # 敵と隣接

- action: load_fake_player_command
  payload:
    command: move_right  # ゴールに向かわず HP が 0 になる経路
```

**確認すべき WorldState 値**:

| キー | 期待値 |
| :--- | :--- |
| `round:result` | `"fail"` |
| `player:hp` | 0 |
| `turn:index` | 1 以上 |

### ステップ 4: continue 経路を確認する

ゴール未到達かつ HP が残存する初期配置で実行する。

```yaml
# continue 経路の boot シーン例
- action: setup_<your_game>_round
  payload:
    player_hp: 3
    enemy_hp:  2
    player_x:  1    # ゴール (4,4) から遠い
    player_y:  1

- action: load_fake_player_command
  payload:
    command: move_right  # 1 手では到達しない
```

**確認すべき WorldState 値**:

| キー | 期待値 |
| :--- | :--- |
| `round:result` | `""` |
| `player:hp` | 1 以上（生存） |
| `turn:index` | 1 |
| シーン遷移 | `boot` に戻る（ループ継続） |

---

## 3. 確認チェックリスト

実装完了の判断基準として以下をすべて満たすこと。

- [ ] `clear` 経路が runtime で確認された
- [ ] `fail` 経路が runtime で確認された
- [ ] `continue` 経路が runtime で確認された（ループして再び boot に戻ること）
- [ ] Debug Overlay で全 WorldState キーが表示される
- [ ] `src/core/` のファイルに変更が加えられていない
- [ ] Validator が不正 payload を受け取ったときエラーを返す
- [ ] pos-0 sacrifice パターンが non-boot シーンに適用されている

---

## 4. 既知の落とし穴

### 4-1. pos-0 スキップバグ

シーン遷移直後は ScenarioRunner が `start_actions()` 後に `step_actions()` を  
呼び出す二重加算により pos-0 がスキップされます。

**回避策**: boot 以外のシーン（clear / fail）の pos-0 に同一 Task のダミーを置く。

```yaml
clear:
  on_enter:
    - action: end_game      # pos-0 ← ダミー（スキップされる）
      payload:
        result: solved
    - action: end_game      # pos-1 ← 実際に実行される
      payload:
        result: solved
```

詳細: `docs/infrastructure_backlog.md`

### 4-2. FakeCommand は smoke 専用

`load_fake_player_command` は本番入力に置き換えること。  
本番では実入力バッファからコマンドを WorldState に書き込む Task を実装する。

### 4-3. continue ループの終了条件

`if_continue: boot` は無限ループになり得ます。  
ゲームデザイン上ループ回数に上限を設ける場合は `turn:index` を threshold として  
`evaluate_round` が判定するように Task を拡張してください。

---

## 5. 参照実装の実測値（roguelike_test、2026-03-21 確認済み）

roguelike_test における全 3 経路の確認済み実測値です。  
新しいゲームのデバッグ時の比較基準として使用してください。

### clear 経路（roguelike_test）

```
player:x = 4, player:y = 4   (start: 4,3 → move_down → 4,4 = goal)
player:hp = 3
enemy_1:hp = 2
round:result = "clear"
turn:index = 1
```

### fail 経路（roguelike_test）

```
player:hp = 0
round:result = "fail"
turn:index = 1+
```

### continue 経路（roguelike_test）

```
player:hp = 1+
enemy_1:hp = 1+
round:result = ""
turn:index = 1
→ boot にループ（シーン遷移なし）
```

---

## 6. 自ゲームの実測値記録欄（ここを埋める）

ゲーム名: _______________  
確認日: _______________

| 経路 | 確認状態 | 実測 `round:result` | 実測 `turn:index` | 備考 |
| :--- | :--- | :--- | :--- | :--- |
| clear | ⬜ 未確認 | | | |
| fail | ⬜ 未確認 | | | |
| continue | ⬜ 未確認 | | | |
