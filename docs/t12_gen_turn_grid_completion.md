# T12 — gen_scenario_turn_grid.py 完了メモ

**フェーズ**: Phase 3-C  
**作成日**: 2026-04-01  
**前提**: `docs/generator_spec_schema.md` T12-1〜T12-8

---

## 1. 実装概要

`tools/gen_scenario_turn_grid.py` — Turn/Grid Basic 専用 Scaffold Generator を実装しました。

| 項目 | 内容 |
|:---|:---|
| テンプレート | `turn_grid_basic` のみ |
| 入力 | Structured Spec YAML |
| 出力 | `scenarios/generated/<name>_<ts>.yaml` + `*_review.md` |
| バリデーション | V-TG-01〜V-TG-09（Fail-Fast） |
| pos-0 sacrifice | terminal 2 シーンのみ（`boot` は不要・R-1 確認済み） |
| 行数 | 約 390 行 |

---

## 2. R-1 / R-4 HG-4 smoke 確認結果

| リスク | 確認方法 | 結果 |
|:---:|:---:|:---|
| R-1: boot→boot ループ時の pos-0 skip | headless smoke × 2 本 | **確定** |
| R-4: apply_player_move の方向渡し | コード読解 + headless smoke | **確定** |

### R-1 詳細

**smoke YAML**: `src/games/roguelike_test/scenario/roguelike_r1_clear_smoke.yaml`（player (2,4)、goal (4,4)、command: move_right）

**実行ログ**:
```
[SetupRoguelikeRoundTask] Round initialised: player_hp=3 enemy_hp=2   ← 反復1: pos-0 実行
[LoadFakePlayerCommandTask] Loaded command: move_right
[ApplyPlayerMoveTask] Player moved to (3,4).
[EvaluateRoguelikeRoundTask] Continue → boot

[LoadFakePlayerCommandTask] Loaded command: move_right                 ← 反復2: pos-0 スキップ（setup なし）
[ApplyPlayerMoveTask] Player moved to (4,4).
[ApplyPlayerMoveTask] Player reached goal → clear.
[EvaluateRoguelikeRoundTask] Clear → clear
[EndGameTask] Game ended with result: solved  ✅
```

**結論**: `boot` に pos-0 sacrifice は**不要**。`setup_roguelike_round`（pos-0）は初回のみ実行され、ループ時はスキップ → 状態保持。これは設計内動作。

### R-4 詳細

`load_fake_player_command.cpp`:
- WorldState に `last_action:type = <command>` を書き込む

`apply_player_move_task.cpp`:
- WorldState から `last_action:type` を読み込み、`dx/dy` を決定
- payload は不要

---

## 3. smoke テスト結果

| ケース | 入力 | 結果 |
|:---|:---|:---:|
| V-TG-01 エラー（template 不正） | `template: branching_basic` | ✅ exit 1 |
| V-TG-07 エラー（first_command 不正） | `first_command: jump` | ✅ exit 1 |
| V-TG-08 エラー（terminal 名重複） | `terminal_clear: same, terminal_fail: same` | ✅ exit 1 |
| V-TG-08 エラー（terminal = boot 予約語） | `terminal_clear: boot` | ✅ exit 1 |
| 最小構成（attack）→ validate exit 0 | `dungeon_minimal` spec | ✅ exit 0 |
| 推奨構成（move_down, 座標あり）→ validate exit 0 | `bamboo_grove_escape` spec | ✅ exit 0 |

---

## 4. Generator の出力仕様

### boot シーン（attack コマンドの場合）

```yaml
boot:
  on_enter:
    - action: setup_roguelike_round   # pos-0: 初回のみ実行
      payload:
        player_hp: N
        enemy_hp:  N
        [player_x: N]
        [player_y: N]
        [enemy_1_x: N]
        [enemy_1_y: N]
    - action: load_fake_player_command
      payload:
        command: attack
    - action: apply_player_attack
      payload:
        target: enemy_1
    - action: apply_enemy_turn
    - action: resolve_roguelike_turn
    - action: evaluate_roguelike_round
      payload:
        if_clear:    <terminal_clear>
        if_fail:     <terminal_fail>
        if_continue: boot
```

### boot シーン（move_* コマンドの場合）

`apply_player_attack` → `apply_player_move` に変わる（payload なし）。

### terminal シーン（pos-0 sacrifice あり）

```yaml
<terminal_clear>:
  on_enter:
    - action: end_game    # pos-0 sacrifice
      payload: {result: solved}
    - action: end_game
      payload: {result: solved}
```

---

## 5. 既知の制限（v1.0 スコープ外）

| 制限 | 詳細 |
|:---|:---|
| enemy 複数対応（R-2） | v1.0 は enemy 1 体のみ。`enemies[0]` の hp を `enemy_hp` に使用 |
| apply_player_attack の target（R-3） | 常に `enemy_1`。`enemies[2〜4]` には対応しない |
| ゴール座標指定 | `setup_roguelike_round` の `goal_x / goal_y` は非対応（デフォルト (4,4) 固定） |

---

## 6. 参照ドキュメント

| ファイル | 役割 |
|:---|:---|
| `docs/generator_spec_schema.md` T12節 | 入口設計・フィールド仕様・バリデーション規則 |
| `docs/infrastructure_backlog.md` | pos-0 skip の known issue 詳細 |
| `src/games/roguelike_test/scenario/roguelike_r1_clear_smoke.yaml` | R-1 確認 smoke YAML |
| `scenarios/generated/turn_grid_basic_expected_output.yaml` | 期待出力サンプル（validate 通過確認済み） |
