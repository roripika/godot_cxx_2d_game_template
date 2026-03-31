# Generator Spec Schema — Branching Basic

**バージョン**: v1.0  
**作成日**: 2026-04-01  
**対象 Generator**: Phase 3-C 初期実装（Branching Basic のみ）  
**前提**: `docs/phase3_c_generator_entry.md` セクション 5・6・7 に基づく

---

## 概要

このファイルは、Generator が受け取る **Structured Spec（構造化仕様）** の入力スキーマを定義する。  
Structured Spec → Generator → YAML 骨格 という変換の入力インターフェース仕様書である。

```
[Structured Spec YAML]          (このスキーマで定義)
         │
         ▼
   [Generator]                  (Phase 3-C で実装)
         │ pos-0 sacrifice 自動配置
         │ __FILL_IN__ 挿入
         ▼
   [YAML 骨格]                   → HG-2 → validate → HG-3 → runtime
```

---

## 1. フィールド仕様

### 1-1. 共通フィールド（全テンプレート共通）

| フィールド | 型 | 必須 | 制約 | 説明 |
|:---|:---:|:---:|:---|:---|
| `template` | string | ✅ | `branching_basic` 固定 | テンプレート識別子 |
| `scenario_name` | string | ✅ | `[a-z0-9_]`、1〜40 文字 | 出力ファイル名のベース |
| `goal.clear_condition` | string | — | 任意の説明文 | 出力 YAML の先頭コメントに埋め込む |
| `goal.fail_condition` | string | — | 任意の説明文 | 出力 YAML の先頭コメントに埋め込む |

### 1-2. Branching Basic 固有フィールド

| フィールド | 型 | 必須 | 制約 | 説明 |
|:---|:---:|:---:|:---|:---|
| `branches` | list | ✅ | 2〜4 要素 | 収集する証拠・フラグの定義 |
| `branches[].id` | string | ✅ | `[a-z0-9_]`、1〜30 文字 | `discover_evidence` の `evidence_id` に使用 |
| `branches[].label` | string | ✅ | 任意の説明文 | review.md のコメント用（YAML には出力しない） |
| `branches[].location` | string | — | `[a-z0-9_]` | `discover_evidence` の `location`。省略時は `id + "_location"` で自動生成 |
| `check_type` | string | ✅ | `all_of` または `any_of` | `check_condition` の判定方式 |
| `scenes.start` | string | — | `[a-z0-9_]` | デフォルト: `investigation` |
| `scenes.terminal_clear` | string | ✅ | `[a-z0-9_]`、`scenes.terminal_fail` と異なる | clear 経路の terminal シーン名 |
| `scenes.terminal_fail` | string | ✅ | `[a-z0-9_]`、`scenes.terminal_clear` と異なる | fail 経路の terminal シーン名 |
| `terminal_result.clear` | string | — | `solved/wrong/failed/lost/timeout` のいずれか | デフォルト: `solved` |
| `terminal_result.fail` | string | — | `solved/wrong/failed/lost/timeout` のいずれか | デフォルト: `wrong` |

---

## 2. 完全な仕様例

### 最小構成（必須フィールドのみ）

```yaml
template: branching_basic
scenario_name: museum_heist

branches:
  - id: fingerprint
  - id: testimony

check_type: all_of

scenes:
  terminal_clear: arrest_scene
  terminal_fail:  release_scene
```

### 推奨構成（オプションフィールドも含む）

```yaml
template: branching_basic
scenario_name: museum_heist

goal:
  clear_condition: |
    指紋と目撃証言が両方揃えば逮捕できる
  fail_condition: |
    どちらか一方でも欠けていたら釈放になる

branches:
  - id: fingerprint
    label: 「指紋」カード
    location: display_case
  - id: testimony
    label: 「目撃証言」カード
    location: security_desk

check_type: all_of

scenes:
  start:          investigation
  terminal_clear: arrest_scene
  terminal_fail:  release_scene

terminal_result:
  clear: solved
  fail:  wrong
```

---

## 3. バリデーション規則（Generator が入力時に検証する）

Generator は Structured Spec を受け取った時点で以下を即時検証する。  
いずれかが失敗した場合は **YAML を生成せず、エラーを返して停止する**（Fail-Fast）。

| 規則 ID | チェック内容 | エラー例 |
|:---|:---|:---|
| `V-01` | `template` が `branching_basic` であること | `"template: roguelike_basic" は未対応` |
| `V-02` | `scenario_name` が `[a-z0-9_]` のみ、1〜40 文字 | `"My Scenario!"` は無効 |
| `V-03` | `branches` が 2〜4 要素であること | 1 要素では check_condition が意味をなさない |
| `V-04` | `branches[].id` が全て `[a-z0-9_]`、1〜30 文字 | `"evidence A"` は空白を含むため無効 |
| `V-05` | `branches[].id` がリスト内で重複しないこと | `fingerprint` が 2 回はエラー |
| `V-06` | `check_type` が `all_of` または `any_of` であること | `"both"` は無効 |
| `V-07` | `scenes.terminal_clear` と `scenes.terminal_fail` が指定されていること | どちらか欠落でエラー |
| `V-08` | `scenes.terminal_clear` ≠ `scenes.terminal_fail` であること | 同名は分岐にならない |
| `V-09` | `scenes.start` が `scenes.terminal_clear` / `terminal_fail` と異なること | start シーンを terminal に指定はエラー |
| `V-10` | `terminal_result.clear/fail` が有効な result 値であること | 有効値: `solved/wrong/failed/lost/timeout` |
| `V-11` | `branches[].location` が指定されている場合 `[a-z0-9_]` であること | `"Display Case"` は無効 |

---

## 4. 入力 → 出力 YAML のマッピング規則

Generator はこのマッピングに従って YAML 骨格を生成する。  
マッピングから外れる変換は Generator に持たせない。

| 入力フィールド | 出力 YAML の該当箇所 | 備考 |
|:---|:---|:---|
| `scenes.start`（デフォルト: `investigation`） | `start_scene:` | トップレベル |
| `branches[].id` + `location` | `discover_evidence` の `evidence_id`: + `location:` | `location` 省略時は `{id}_location` で自動生成 |
| `check_type` | `check_condition` の `all_of:` / `any_of:` キー | |
| `branches[].id` （全件） | `check_condition` の条件リスト | `- evidence: {id}` を全件追加 |
| `scenes.terminal_clear` | `check_condition` の `if_true:` + シーン名 | |
| `scenes.terminal_fail` | `check_condition` の `if_false:` + シーン名 | |
| `terminal_result.clear`（デフォルト: `solved`） | terminal_clear シーンの `end_game.result:` | pos-0 sacrifice × 2 を自動配置 |
| `terminal_result.fail`（デフォルト: `wrong`） | terminal_fail シーンの `end_game.result:` | pos-0 sacrifice × 2 を自動配置 |
| `goal.*` | 出力 YAML の先頭コメント行 | `# goal: ...` として埋め込む |
| `branches[].label` | 出力しない（review.md のみに記載） | |

### pos-0 sacrifice の自動配置ルール

- `start` シーン（investigation）: sacrifice **不要**
- `terminal_clear` シーン: pos-0 に同一 Task を **2 回** 配置（1 回目 sacrifice、2 回目実処理）
- `terminal_fail` シーン: 同上

この規則は **Generator が無条件に適用する**。HG-2 で人間が目視確認する。

---

## 5. 期待出力サンプル

**ファイル**: `scenarios/generated/branching_basic_expected_output.yaml`

Shot 1（`docs/few_shot_prompts.md`）の YAML を期待出力サンプルとして確定する。  
このファイルは **Generator の出力品質基準** として使用する。

- `validate_scenario.py` で exit 0 確認済み（Phase 3-B T10 で検証）
- 上記「推奨構成」の入力に対応する出力
- Generator の出力がこの形式に一致することを手動で確認する（HG-2）

```
scenarios/generated/
├── .gitkeep                              # ディレクトリ確保（空ファイル）
└── branching_basic_expected_output.yaml  # 期待出力サンプル（バージョン管理対象）
```

> **注意**: Generator が実際に生成するファイル（`<scenario_name>_<timestamp>.yaml`）は  
> 人間が HG-2〜HG-4 を通過させるまでバージョン管理対象にしない。  
> `git add` は HG-4 通過後に人間が手動で行う。

---

## 6. スキーマの変更プロセス

このスキーマを変更するには以下の条件を満たすこと。

1. `docs/phase3_c_generator_entry.md` の Out of Scope に違反していないこと
2. 変更前後で `scenarios/generated/branching_basic_expected_output.yaml` の  
   `validate_scenario.py` exit 0 が維持されること
3. 変更は `docs/generator_spec_schema.md` と `docs/README.md` を同時に更新し commit すること

---

## 参照ドキュメント

| ファイル | 役割 |
|:---|:---|
| `docs/phase3_c_generator_entry.md` | Generator 入口設計（このスキーマの親ドキュメント） |
| `docs/task_catalog.md` | `discover_evidence` / `check_condition` / `end_game` の payload 仕様 |
| `docs/few_shot_prompts.md` | Shot 1（期待出力サンプルの出典） |
| `docs/validate_scenario_guidance.md` | HG-3 向けエラー対処表 |
| `scenarios/generated/branching_basic_expected_output.yaml` | 期待出力サンプル（validate 通過確認済み） |

---

---

# Turn/Grid Basic — Generator 入口設計

**ステータス**: 設計確定・実装未着手（Phase 3-C）  
**バージョン**: v1.0-entry  
**前提**: `docs/roguelike_test_design.md` / `docs/roguelike_test_completion.md`

---

## T12-1. 概要

Turn/Grid Basic 用 Generator は Branching Basic（T11）の次に実装するスキャフォールドジェネレーターである。  
入力として **Structured Spec** を受け取り、`setup_roguelike_round / load_fake_player_command / apply_player_attack or apply_player_move / apply_enemy_turn / resolve_roguelike_turn / evaluate_roguelike_round` を含む YAML 骨格を生成する。

```
[Structured Spec YAML]
         │  template: turn_grid_basic
         ▼
   [gen_scenario_turn_grid.py]     (T12 で実装する想定)
         │ pos-0 sacrifice 自動配置（clear / fail シーン）
         │ __FILL_IN__ 挿入（省略フィールド）
         ▼
   [YAML 骨格]   → HG-2 → validate_scenario.py → HG-3 → runtime → HG-4
```

---

## T12-2. 入力フィールド仕様

### 2-1. 共通フィールド（Branching Basic と同一）

| フィールド | 型 | 必須 | 説明 |
|:---|:---:|:---:|:---|
| `template` | string | ✅ | 固定値 `"turn_grid_basic"` |
| `scenario_name` | string | ✅ | YAML ファイル名の一部（英数字・アンダースコアのみ） |
| `description` | string | — | 人間向けコメント（YAML ヘッダに出力） |

### 2-2. ターン制固有フィールド

| フィールド | 型 | 必須 | 説明 |
|:---|:---:|:---:|:---|
| `player.hp` | int | ✅ | プレイヤー初期 HP（1〜10） |
| `player.start_x` | int | — | プレイヤー初期 X 座標（0〜9。省略時はデフォルト） |
| `player.start_y` | int | — | プレイヤー初期 Y 座標（0〜9。省略時はデフォルト） |
| `enemies` | list | ✅ | 1〜4 体のリスト |
| `enemies[].id` | string | ✅ | `enemy_1`〜`enemy_4` 形式 |
| `enemies[].hp` | int | ✅ | 敵初期 HP（1〜10） |
| `enemies[].x` | int | — | 敵初期 X 座標（0〜9。省略時はデフォルト） |
| `enemies[].y` | int | — | 敵初期 Y 座標（0〜9。省略時はデフォルト） |
| `first_command` | string | ✅ | プレイヤーの最初の行動（下表参照） |
| `terminal_clear` | string | ✅ | クリア時の遷移先シーン名 |
| `terminal_fail` | string | ✅ | 失敗時の遷移先シーン名 |

**`first_command` 許可値**:

| 値 | 生成される Task |
|:---|:---|
| `attack` | `apply_player_attack` `{target: enemy_1}` |
| `move_up` | `apply_player_move`（payload なし） |
| `move_down` | `apply_player_move`（payload なし） |
| `move_left` | `apply_player_move`（payload なし） |
| `move_right` | `apply_player_move`（payload なし） |

---

## T12-3. バリデーション規則

| 規則 ID | 対象 | 内容 |
|:---|:---|:---|
| V-TG-01 | `template` | `"turn_grid_basic"` と完全一致すること |
| V-TG-02 | `scenario_name` | 英数字・アンダースコアのみ。空文字・スペース禁止 |
| V-TG-03 | `player.hp` | 1〜10 の整数 |
| V-TG-04 | `enemies` | 1〜4 要素のリスト |
| V-TG-05 | `enemies[].id` | `enemy_[1-4]` パターン。重複不可 |
| V-TG-06 | `enemies[].hp` | 1〜10 の整数 |
| V-TG-07 | `first_command` | `attack/move_up/move_down/move_left/move_right` のいずれか |
| V-TG-08 | `terminal_clear` / `terminal_fail` | 互いに異なること。空文字禁止 |
| V-TG-09 | 座標フィールド | 0〜9 の整数（省略は許可） |

---

## T12-4. Spec → YAML マッピング規則

### 4-1. boot シーン（生成される Task 列）

```
setup_roguelike_round
    player_hp:  <player.hp>
    enemy_hp:   <enemies[0].hp>            # v1.0: 1 体目の hp を採用
    [player_x:  <player.start_x>]          # 省略可
    [player_y:  <player.start_y>]          # 省略可
    [enemy_1_x: <enemies[0].x>]            # 省略可
    [enemy_1_y: <enemies[0].y>]            # 省略可

load_fake_player_command
    command: <first_command>

--- first_command が "attack" の場合 ---
apply_player_attack
    target: enemy_1                        # v1.0: 常に enemy_1（先頭のみ）

--- first_command が "move_*" の場合 ---
apply_player_move                          # payload なし

apply_enemy_turn                           # payload なし
resolve_roguelike_turn                     # payload なし

evaluate_roguelike_round
    if_clear:    <terminal_clear>
    if_fail:     <terminal_fail>
    if_continue: boot                      # 固定（ループで boot に戻る）
```

### 4-2. terminal シーン（clear / fail — pos-0 sacrifice 適用）

```yaml
<terminal_clear>:
  on_enter:
    - action: end_game          # pos-0 sacrifice（スキップされる）
      payload: {result: solved}
    - action: end_game          # 実際に実行される
      payload: {result: solved}

<terminal_fail>:
  on_enter:
    - action: end_game          # pos-0 sacrifice（スキップされる）
      payload: {result: failed}
    - action: end_game          # 実際に実行される
      payload: {result: failed}
```

> **確定（R-1 HG-4 smoke 済み）**: `boot` に **pos-0 sacrifice は不要**。  
> - 初回（`_ready()` 経由 `load_scene_by_id`）: index=0 から開始するため `setup_roguelike_round` が正常実行される。  
> - ループ 2 回目以降（`evaluate_roguelike_round` → `boot`）: `load_scene_by_id` から `start_actions`(index=0) → 呼び出しタスクが Success を返す → `step_actions` が index++ → `setup_roguelike_round`（pos-0）がスキップされ、WorldState の状態（player 座標・HP）が保持される。**これは仕様通りの動作**。  
> - terminal シーン（clear / fail）は `load_scene_by_id` から直接遷移するため pos-0 sacrifice が必要（従来通り）。

---

## T12-5. Branching Basic との差異

| 観点 | Branching Basic | Turn/Grid Basic |
|:---|:---|:---|
| シーン構造 | 線形（start → terminal） | ループあり（boot → boot） |
| FakeCommand | 不要 | 必須（`load_fake_player_command`） |
| エンティティ | エビデンス/条件のみ | Player + Enemy（複数対応） |
| 結果分岐 | 2 経路（true/false） | 3 経路（clear/fail/continue） |
| pos-0 sacrifice | terminal 2 シーン | terminal 2 シーン のみ（boot は不要・確定） |
| Task 数 | 3〜5（シーン構成による） | 6〜7 固定 |

---

## T12-6. リスク解決状況

| # | リスク | 状態 | 結果 |
|:---:|:---|:---:|:---|
| R-1 | `evaluate_roguelike_round` → `boot` ループ時の pos-0 skip 発生有無 | ✅ **確定** | pos-0 skip は発生する。`setup_roguelike_round`（pos-0）がループ時にスキップされ WorldState の状態が保持される。`boot` に sacrifice **不要**。smoke YAML: `roguelike_r1_clear_smoke.yaml` |
| R-2 | 複数 enemy（2〜4 体）の `setup_roguelike_round` payload 仕様 | ⚪ **v1.0 スコープ外** | v1.0 は enemy 1 体のみ対応。`enemy_hp` フィールドのみ使用。 |
| R-3 | `apply_player_attack.target` に `enemy_2` 以降を指定できるか | ⚪ **v1.0 スコープ外** | v1.0 は `target: enemy_1` 固定。 |
| R-4 | `apply_player_move` の方向パラメータ渡し | ✅ **確定** | `load_fake_player_command` が `WorldState(last_action:type)` に書き込み、`apply_player_move` がそれを読む。`apply_player_move` に payload **不要**。コード + HG-4 smoke で確認済み。 |

---

## T12-7. 最小 Spec 例

```yaml
template: turn_grid_basic
scenario_name: dungeon_escape

player:
  hp: 3
  start_x: 1
  start_y: 1

enemies:
  - id: enemy_1
    hp: 2
    x: 2
    y: 1

first_command: attack

terminal_clear: victory
terminal_fail:  defeat
```

### 期待出力 YAML 骨格

```yaml
start_scene: boot

scenes:

  boot:
    on_enter:
      - action: setup_roguelike_round
        payload:
          player_hp: 3
          enemy_hp:  2
          player_x:  1
          player_y:  1
          enemy_1_x: 2
          enemy_1_y: 1

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
          if_clear:    victory
          if_fail:     defeat
          if_continue: boot

  victory:
    on_enter:
      - action: end_game          # pos-0 sacrifice
        payload: {result: solved}
      - action: end_game
        payload: {result: solved}

  defeat:
    on_enter:
      - action: end_game          # pos-0 sacrifice
        payload: {result: failed}
      - action: end_game
        payload: {result: failed}
```

---

## T12-8. 実装予定ファイル

| ファイル | 役割 |
|:---|:---|
| `tools/gen_scenario_turn_grid.py` | Turn/Grid Basic Scaffold Generator 本体 |
| `scenarios/generated/turn_grid_basic_expected_output.yaml` | 期待出力サンプル（T12 実装後に追加） |
| `docs/t12_gen_turn_grid_completion.md` | T12 完了メモ（T12 実装後に作成） |

> **実装制約**: この節（T12-1〜T12-8）は設計確定済みだが、  
> **R-1 / R-4 のリスクを HG-4 smoke で解消してから Generator 本体を実装すること。**
