# roguelike_test 設計・仕様ドキュメント v1.0

Date: 2026-03-21

---

## 1. roguelike_test の目的

`roguelike_test` は、ローグライクゲームを完成させるための module ではない。

その役割は、**ターン制・エンティティ・グリッドを伴う module が Kernel v2.0 上で
安全に成立するかを検証する最小 fitness test** である。

具体的には、以下の問いに答えることが目的である。

- ターン構造（player → enemy → resolve）を YAML + Task で扱えるか
- 複数エンティティ（player / enemy / goal）のグリッド座標を WorldState で安全に管理できるか
- clear / fail / continue の 3 分岐が ScenarioRunner の if_clear / if_fail / if_continue 経由で正しく機能するか
- `src/core` を変更せずに新しい genre の module を追加できるか

---

## 2. mystery_test / billiards_test との違い

| 観点 | mystery_test | billiards_test | roguelike_test |
|---|---|---|---|
| 主たる検証 | dialogue / evidence / conditional の基準実装 | event-driven 外部流入の耐性 | turn-based / entity / grid の最小成立確認 |
| 状態駆動 | flag 中心（証拠品収集・HP） | event 待機（ball_pocketed）| 座標 + HP + ターンフェーズ |
| フロー制御 | 線形 + 条件分岐 | 外部イベント待機 + 分岐 | ターン制ループ + 3-way 分岐 |
| fake 方針 | action 経由で state を直接操作 | fake event を inject してコールバック模倣 | fake command を WorldState に事前書き込み |
| src/core 変更 | なし | なし | なし |

---

## 3. 最小ゲーム仕様

### マップ

- 固定 5×5 グリッド
- セルの種類: 床（到達可能）のみ
- 壁: マップ外境界のみ（`[0,4]` が有効範囲）

### エンティティ（現時点）

| 識別子 | 説明 |
|---|---|
| player | 1 体。fake command を受けて移動 / 攻撃する |
| enemy_1 | 1 体。隣接時に攻撃、非隣接時にプレイヤーへ接近する |
| goal | 1 マス。プレイヤーが踏んだら clear |

### 勝敗条件

| 条件 | 結果 |
|---|---|
| player が goal マスに移動 | `round:result = "clear"` |
| player:hp ≦ 0 | `round:result = "fail"` |
| どちらも未達成 | `round:result = ""`（continue） |

---

## 4. WorldState contract

### namespace

すべてのキーは namespace `"roguelike_test"`、scope `SCOPE_SESSION (= 1)` に属する。

### キー一覧

| WorldState キー | 型 | デフォルト | 説明 |
|---|---|---|---|
| `turn:index` | int | 0 | 消化済みターン数 |
| `turn:phase` | String | `"player"` | 現在フェーズ (`"player"` / `"enemy"` / `"resolve"`) |
| `player:x` | int | 1 | プレイヤー X 座標 |
| `player:y` | int | 1 | プレイヤー Y 座標 |
| `player:hp` | int | 3 | プレイヤー HP |
| `enemy_1:x` | int | 3 | enemy_1 X 座標 |
| `enemy_1:y` | int | 3 | enemy_1 Y 座標 |
| `enemy_1:hp` | int | 2 | enemy_1 HP |
| `goal:x` | int | 4 | ゴール X 座標 |
| `goal:y` | int | 4 | ゴール Y 座標 |
| `round:result` | String | `""` | ラウンド結果 (`""` / `"clear"` / `"fail"`) |
| `last_action:actor` | String | `""` | 最終アクションの主体 (`"player"` / `"enemy_1"`) |
| `last_action:type` | String | `""` | 最終アクションの種別 (`"move"` / `"attack"` / `"wait"`) |

### ルール

- WorldState には **論理状態のみ** 格納する
- 描画座標・スプライト位置・アニメーション状態は保存しない
- 毎フレーム更新される一時情報は保存しない
- `SCOPE_SCENE` (= 2) は使用しない。シーン遷移で消えると困る情報は `SCOPE_SESSION` を使う

---

## 5. scene / turn 構造

### YAML シーン構成

```
start_scene: boot

scenes:
  boot:
    on_enter:
      - setup_roguelike_round      # WorldState 13 キーを初期化
      - load_fake_player_command   # fake command を WorldState に書き込む
      - apply_player_move          # または apply_player_attack
      - apply_enemy_turn           # 敵の行動（隣接→攻撃、非隣接→接近）
      - resolve_roguelike_turn     # turn:index++、phase / last_action リセット
      - evaluate_roguelike_round   # clear / fail / continue の分岐先へジャンプ

  clear:
    on_enter:
      - end_game { result: solved }

  fail:
    on_enter:
      - end_game { result: failed }
```

### 1 ターンのフェーズ順序

```
player コマンド決定（fake command 書き込み）
    ↓
apply_player_move / apply_player_attack
    ↓
apply_enemy_turn
    ↓
resolve_roguelike_turn（turn:index++、phase 初期化）
    ↓
evaluate_roguelike_round（clear → if_clear / fail → if_fail / else → if_continue）
```

---

## 6. fake command 方針

### 目的

real input（キーボード・コントローラー）を接続せずに、
コマンドの processing パスを smoke test できるようにする。

### 仕組み

1. YAML の `load_fake_player_command` action が payload で command 名を受け取る
2. `LoadFakePlayerCommandTask` が `last_action:type` に command 名を書き込む
3. 後続の `ApplyPlayerMoveTask` / `ApplyPlayerAttackTask` がその値を読んで処理する

### 有効な command 名（現時点）

`move_up` / `move_down` / `move_left` / `move_right` / `attack`

### 方針

- fake command は deterministic であること。同じ YAML は常に同じ結果になる
- fake command は runtime に変化しない（入力受付や乱数は介在しない）
- この方針は *smoke test のための手段* であり、real input への置き換えを妨げない

---

## 7. debug view の役割

### 位置づけ

`samples/roguelike/roguelike_debug_scene.tscn` / `roguelike_debug_overlay.gd` は
**完成 UI ではない**。

その役割は以下に限定される。

- WorldState の全 13 キーが正しく書き込まれていることを目視確認する **observer**
- mystery シェルを経由せずに roguelike_test が独立して動作することを確認する **smoke 実行環境**
- `last_action` が毎ターン更新されていることをリアルタイムで確認する **diagnostic overlay**

### 実装

- `CanvasLayer (layer=10)` の `Control` ノードとして乗せる
- `_process()` 毎フレームで WorldState から全キーを読み取り、Label に出力する
- 左側: 5×5 ASCII グリッド（`P`=player / `E`=enemy / `G`=goal / `.`=床）
- 右側: 13 キーの値一覧

### 制約

- ゲームロジックへの影響はゼロ（読み取り専用）
- `src/core` への変更なし
- Task / YAML の変更なし

---

## 8. smoke / validator coverage

### smoke シナリオ（3 経路）

| ファイル | 検証経路 | 期待結果 |
|---|---|---|
| `roguelike_clear_smoke.yaml` | player(4,3) → move_down → goal(4,4) 到達 | `round:result = "clear"` |
| `roguelike_fail_smoke.yaml` | player hp=1、enemy 隣接→攻撃→ player hp=0 | `round:result = "fail"` |
| `roguelike_continue_smoke.yaml` | player 移動、enemy 接近、勝敗未決 | `round:result = ""`（continue） |

### negative fixture

| ファイル | 内容 |
|---|---|
| `roguelike_corrupted.yaml` | 5 つの意図的エラー。validate_scenario.py で reject されることを確認 |

### validator coverage（tools/validate_scenario.py）

- roguelike 専用 7 アクション名の定義済み列挙
- `load_fake_player_command` の command 値検証
- `evaluate_roguelike_round` の if_clear / if_fail / if_continue の有効 scene 参照確認
- `setup_roguelike_round` の payload キー検証

### test 結果

```
python3 tools/test_validator.py  →  ALL 13 TESTS PASSED
```

---

## 9. 今やらないこと

以下は現時点では対象外である。将来の機能として扱う。

| 項目 | 理由 |
|---|---|
| real input 統合 | smoke test の範囲外。fake command で十分 |
| 手続き的生成（procedural generation） | fixed map で fitness test として成立している |
| 複数 enemy の一般化 | `enemy_1` 1 体で検証目的を満たす |
| FOV / 視野マスク | ロジック段階では不要 |
| inventory / item | roguelike_test の scope 外 |
| 完成 UI・アニメーション | debug view で十分 |
| save / load | SCOPE_SESSION のみで完結している |

---

## 10. 次段階へ進む条件

以下がすべて満たされた場合、roguelike_test は「成立」とみなし、
次の module または機能に進むことができる。

1. **smoke 3 経路が validator + runtime で決定論的に通ること**  
   → 現時点で validator は PASS。runtime 確認は Godot 実行後に行う。

2. **WorldState の 13 キーが debug view で正しく可視化されること**  
   → `roguelike_debug_scene.tscn` を Godot で開いて肉眼確認する。

3. **`src/core` への変更がゼロであること**  
   → コミット差分で確認。

4. **corrupted scenario が validator で reject されること**  
   → `ALL 13 TESTS PASSED` で確認済み。

上記を満たした後、次は以下のいずれかへ進む。

- real input 統合（optional）
- 別 genre の fitness test 追加
- Template / Assist / Generator 設計（Phase 3）

ただし、**本ドキュメントの対象外事項（§9）に踏み込む判断は、明示的な意思決定を経てから行う**こと。
