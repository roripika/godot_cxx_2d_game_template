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

---

---

# Time/Clock Basic — Generator 入口設計

**ステータス**: 設計確定・実装未着手（Phase 3-C）  
**バージョン**: v1.0-entry  
**前提**: `docs/rhythm_test_design.md` / `docs/rhythm_test_completion.md`

---

## T13-1. 概要

Time/Clock Basic 用 Generator は Turn/Grid Basic（T12）の次に実装するスキャフォールドジェネレーターである。  
`rhythm_test` モジュールの Task 群（`setup_rhythm_round` / `advance_rhythm_clock` / `load_fake_tap` / `judge_rhythm_note` / `resolve_rhythm_progress` / `evaluate_rhythm_round`）を使い、**時間軸同期型の判定ループ**を持つ YAML 骨格を生成する。

```
[Structured Spec YAML]
         │  template: time_clock_basic
         ▼
   [gen_scenario_time_clock.py]     (T13 で実装)
         │ boot:             pos-0 sacrifice 不要（_ready() 起動）
         │ 中間シーン:       pos-0 sacrifice 必要（×2 配置）
         │ terminal:         pos-0 sacrifice 必要（×2 配置）
         │ __FILL_IN__ 挿入（省略フィールド）
         ▼
   [YAML 骨格]   → HG-2 → validate_scenario.py → HG-3 → runtime → HG-4
```

### シーン構成（固定 4 中間シーン + 2 terminal）

```
boot
 └──(evaluate → if_continue)──▶ advance
                                   └──(evaluate → if_continue)──▶ judge
                                                                      └──(evaluate → if_continue)──▶ resolve
                                                                                                        └──(evaluate → if_continue)──▶ loop_gate
                                                                                                                                          └──(evaluate → if_continue)──▶ advance  ← ループ
                                                           ↓ if_clear                                ↓ if_clear                              ↓ if_clear
                                                         terminal_clear                           terminal_clear                         terminal_clear
                                                           ↓ if_fail                                 ↓ if_fail                               ↓ if_fail
                                                         terminal_fail                            terminal_fail                          terminal_fail
```

---

## T13-2. 入力フィールド仕様

### 2-1. 共通フィールド

| フィールド | 型 | 必須 | 説明 |
|:---|:---:|:---:|:---|
| `template` | string | ✅ | 固定値 `"time_clock_basic"` |
| `scenario_name` | string | ✅ | YAML ファイル名の一部（英数字・アンダースコアのみ） |
| `description` | string | — | 人間向けコメント（YAML ヘッダに出力） |

### 2-2. 譜面・タイミング固有フィールド

| フィールド | 型 | 必須 | デフォルト | 説明 |
|:---|:---:|:---:|:---:|:---|
| `notes` | list(int) | ✅ | — | ノーツ時刻 ms（3〜5 要素、正数、厳密昇順） |
| `taps` | list(int) | ✅ | — | タップ時刻 ms（`notes` と同数、-1 = タップなし） |
| `advance_ms` | int | — | `1000` | 1 ステップごとのクロック進行 ms |
| `perfect_window_ms` | int | — | `50` | perfect 判定幅 ms |
| `good_window_ms` | int | — | `150` | good 判定幅 ms（`>= perfect_window_ms`） |
| `clear_hit_count` | int | — | `notes.length` | クリアに必要な hit 数（perfect + good） |
| `max_miss_count` | int | — | `1` | 許容 miss 数（0 = 1 miss で即 fail） |

### 2-3. シーン名フィールド

| フィールド | 型 | 必須 | 制約 | 説明 |
|:---|:---:|:---:|:---|:---|
| `scenes.terminal_clear` | string | ✅ | 予約語禁止・`terminal_fail` と異なる | クリア時の遷移先シーン名 |
| `scenes.terminal_fail` | string | ✅ | 予約語禁止・`terminal_clear` と異なる | 失敗時の遷移先シーン名 |
| `terminal_result.clear` | string | — | 有効 result 値 | デフォルト: `solved` |
| `terminal_result.fail` | string | — | 有効 result 値 | デフォルト: `failed` |

**予約済み内部シーン名**（`terminal_clear` / `terminal_fail` に使用不可）:  
`boot`, `advance`, `judge`, `resolve`, `loop_gate`

---

## T13-3. バリデーション規則

| 規則 ID | 対象 | 内容 |
|:---|:---|:---|
| V-TC-01 | `template` | `"time_clock_basic"` と完全一致すること |
| V-TC-02 | `scenario_name` | `[a-z0-9_]` のみ。空文字・スペース禁止 |
| V-TC-03 | `notes` | 3〜5 要素のリスト。各要素は正整数（> 0）かつ厳密昇順 |
| V-TC-04 | `taps` | `notes` と同数。各要素は整数 `>= -1`（-1 = タップなし） |
| V-TC-05 | `advance_ms` | 正整数（> 0） |
| V-TC-06 | `clear_hit_count` | 1 〜 `len(notes)` の範囲内。省略時は `len(notes)` |
| V-TC-07 | `max_miss_count` | 0 以上の整数。省略時は `1` |
| V-TC-08 | `scenes.terminal_clear/fail` | 両方指定必須。互いに異なること。空文字禁止 |
| V-TC-09 | `scenes.terminal_clear/fail` | 予約済みシーン名（`boot/advance/judge/resolve/loop_gate`）は使用不可 |
| V-TC-10 | `terminal_result.clear/fail` | 省略可。指定時は `solved/wrong/failed/lost/timeout` のいずれか |
| V-TC-11 | `perfect_window_ms` | 正整数（> 0） |
| V-TC-12 | `good_window_ms` | `>= perfect_window_ms` |

---

## T13-4. Spec → YAML マッピング規則

### 4-1. boot シーン

```
setup_rhythm_round
    notes:             <notes>
    taps:              <taps>
    advance_ms:        <advance_ms>          # 省略可（デフォルト 1000）
    [perfect_window_ms: <perfect_window_ms>] # 省略可
    [good_window_ms:    <good_window_ms>]     # 省略可
    [clear_hit_count:   <clear_hit_count>]   # 省略可
    [max_miss_count:    <max_miss_count>]    # 省略可

evaluate_rhythm_round
    if_clear:    <terminal_clear>
    if_fail:     <terminal_fail>
    if_continue: advance                     # 固定
```

> **R-1 準拠（確定）**: `boot` に **pos-0 sacrifice 不要**。  
> `_ready()` 経由の起動では `start_actions(0)` が pos-0 から正常実行するため。

### 4-2. 中間シーン（全シーンに pos-0 sacrifice 必要）

タスク内から `evaluate_rhythm_round` → `load_scene_by_id()` が呼ばれる → `execute()` が Success を返す → `step_actions()` while ループが `pending_action_index_++` → pos-0 がスキップ。  
**`boot` 以外の全シーンで pos-0 sacrifice（同一タスクの複製）が必要**。

#### advance シーン

```yaml
advance:
  on_enter:
    - action: advance_rhythm_clock  # pos-0 sacrifice（スキップされる）
    - action: advance_rhythm_clock  # 実際に実行される
    - action: evaluate_rhythm_round
      payload:
        if_clear:    <terminal_clear>
        if_fail:     <terminal_fail>
        if_continue: judge
```

#### judge シーン

```yaml
judge:
  on_enter:
    - action: load_fake_tap          # pos-0 sacrifice
    - action: load_fake_tap          # 実際に実行される
    - action: judge_rhythm_note
    - action: evaluate_rhythm_round
      payload:
        if_clear:    <terminal_clear>
        if_fail:     <terminal_fail>
        if_continue: resolve
```

#### resolve シーン

```yaml
resolve:
  on_enter:
    - action: resolve_rhythm_progress  # pos-0 sacrifice
    - action: resolve_rhythm_progress  # 実際に実行される
    - action: evaluate_rhythm_round
      payload:
        if_clear:    <terminal_clear>
        if_fail:     <terminal_fail>
        if_continue: loop_gate
```

#### loop_gate シーン（ループ継続判定）

```yaml
loop_gate:
  on_enter:
    - action: evaluate_rhythm_round  # pos-0 sacrifice
      payload:
        if_clear:    <terminal_clear>
        if_fail:     <terminal_fail>
        if_continue: advance
    - action: evaluate_rhythm_round  # 実際に実行される
      payload:
        if_clear:    <terminal_clear>
        if_fail:     <terminal_fail>
        if_continue: advance
```

### 4-3. terminal シーン（clear / fail）

```yaml
<terminal_clear>:
  on_enter:
    - action: end_game          # pos-0 sacrifice（スキップされる）
      payload: {result: <terminal_result.clear>}
    - action: end_game          # 実際に実行される
      payload: {result: <terminal_result.clear>}

<terminal_fail>:
  on_enter:
    - action: end_game          # pos-0 sacrifice（スキップされる）
      payload: {result: <terminal_result.fail>}
    - action: end_game          # 実際に実行される
      payload: {result: <terminal_result.fail>}
```

> **R-5 確定（headless HG-4 smoke 済み）**: terminal シーンも `load_scene_by_id` 経由で遷移するため pos-0 sacrifice が必要。  
> `rhythm_r5_clear_smoke.yaml` / `rhythm_r5_fail_smoke.yaml` で `[EndGameTask] Game ended with result: solved/failed` を確認済み。

---

## T13-5. Turn/Grid Basic との差異

| 観点 | Turn/Grid Basic (T12) | Time/Clock Basic (T13) |
|:---|:---|:---|
| ループ構造 | 単一 boot ループ | 4 中間シーン（advance/judge/resolve/loop_gate）ループ |
| pos-0 sacrifice | boot 不要 / terminal 必要 | boot 不要 / **全中間シーン + terminal 必要** |
| Task 数 / シーン | boot: 5〜7, terminal: 2 | boot: 2, 中間: 各 2〜4, terminal: 2 |
| 時間軸 | なし（ターン制） | `KernelClock` による ms 単位の時間管理 |
| 入力モデル | `load_fake_player_command` → WorldState | `load_fake_tap` → WorldState |
| 結果分岐 | 3 経路（clear/fail/continue） | 3 経路（clear/fail/continue）× 4 シーン |

---

## T13-6. リスク解決状況

| # | リスク | 状態 | 結果 |
|:---:|:---|:---:|:---|
| R-5 | `clear`/`fail` terminal シーンで pos-0 skip が発生し `end_game` が実行されないか | ✅ **確定** | pos-0 skip 発生する。terminal に pos-0 sacrifice **必要**。pos-1 の `end_game` が正常実行され、`[EndGameTask] Game ended with result: solved/failed` を確認。smoke YAML: `rhythm_r5_clear_smoke.yaml` / `rhythm_r5_fail_smoke.yaml` |
| R-6 | `advance`/`judge`/`resolve`/`loop_gate` 各中間シーンで sacrifice パターンが正しく動作するか | ✅ **確定** | 全中間シーン pos-0 sacrifice 必要・正常動作。3 ノーツ = 3 サイクル（advance→judge→resolve→loop_gate）を完走し `rhythm_clear` に到達。headless 実行で確認。 |
| R-7 | headless の `_process()` frame delta が `advance_rhythm_clock` の `advance_ms` と干渉して `load_fake_tap` の `scheduled_tap <= now_ms` 判定を狂わせるか | ✅ **確定** | 干渉なし。headless での frame delta は `advance_ms` 進行量に対して無視できる範囲。`taps=[1000,2000,3000]` / `notes=[1000,2000,3000]` / `advance_ms=1000` で全 perfect 判定 → `result: solved` 確認。 |

また、`scenario_runner.cpp` のコード解析により以下の動作が確定した:
- `boot` 以外の全シーンは `evaluate_rhythm_round::execute()` → `load_scene_by_id()` → `execute()` returns Success → `step_actions()` while ループが `pending_action_index_++` → pos-0 がスキップされる
- これは Turn/Grid Basic（R-1 確定）と同一のメカニズム

---

## T13-7. 最小 Spec 例

```yaml
template: time_clock_basic
scenario_name: rhythm_minimal

notes: [1000, 2000, 3000]
taps:  [1000, 2000, 3000]

scenes:
  terminal_clear: rhythm_clear
  terminal_fail:  rhythm_fail
```

（`advance_ms`=1000 / `clear_hit_count`=3 / `max_miss_count`=1 が暗黙デフォルト）

### 期待出力 YAML 骨格（抜粋）

```yaml
start_scene: boot

scenes:
  boot:
    on_enter:
      - action: setup_rhythm_round
        payload:
          notes: [1000, 2000, 3000]
          taps:  [1000, 2000, 3000]
      - action: evaluate_rhythm_round
        payload:
          if_clear:    rhythm_clear
          if_fail:     rhythm_fail
          if_continue: advance

  advance:
    on_enter:
      - action: advance_rhythm_clock  # pos-0 sacrifice
      - action: advance_rhythm_clock
      - action: evaluate_rhythm_round
        payload:
          if_clear:    rhythm_clear
          if_fail:     rhythm_fail
          if_continue: judge

  # ... judge / resolve / loop_gate （各シーン pos-0 sacrifice 適用）

  rhythm_clear:
    on_enter:
      - action: end_game              # pos-0 sacrifice
        payload: {result: solved}
      - action: end_game
        payload: {result: solved}

  rhythm_fail:
    on_enter:
      - action: end_game              # pos-0 sacrifice
        payload: {result: failed}
      - action: end_game
        payload: {result: failed}
```

---

## T13-8. 実装予定ファイル

| ファイル | 役割 |
|:---|:---|
| `tools/gen_scenario_time_clock.py` | Time/Clock Basic Scaffold Generator 本体 |
| `scenarios/generated/time_clock_basic_expected_output.yaml` | 期待出力サンプル（T13 実装後に追加） |
| `docs/t13_gen_time_clock_completion.md` | T13 完了メモ（T13 実装後に作成） |

> **実装制約**: この節（T13-1〜T13-8）は設計確定済み。  
> **R-5 / R-6 / R-7 はいずれも headless HG-4 smoke で確認済み**（2026-04-01）。Generator 本体の実装に進める。

---

---

# Event-Driven Basic — Generator 入口設計

**ステータス**: 設計確定・実装未着手（Phase 3-C）  
**バージョン**: v1.0-entry  
**前提**: `docs/billiards_test_completion.md`

---

## T14-1. 概要

Event-Driven Basic 用 Generator は Time/Clock Basic（T13）の次に実装するスキャフォールドジェネレーターである。  
`billiards_test` モジュールの Task 群（`setup_billiards_round` / `wait_for_billiards_event` / `record_billiards_event` / `evaluate_billiards_round`）を使い、**`TaskResult::Waiting` によるイベント待機ループ**を持つ YAML 骨格を生成する。

```
[Structured Spec YAML]
         │  template: event_driven_basic
         ▼
   [gen_scenario_event_driven.py]     (T14 で実装)
         │ boot(setup_round): pos-0 sacrifice 不要（_ready() 起動）
         │ continue(shoot_again): pos-0 sacrifice 必要（×2 配置）
         │ terminal:              pos-0 sacrifice 必要（×2 配置）
         │ __FILL_IN__ 挿入（省略フィールド）
         ▼
   [YAML 骨格]   → HG-2 → validate_scenario.py → HG-3 → runtime → HG-4
```

### シーン構成（boot + continue ループ + 2 terminal）

```
setup_round  (boot: _ready() 起動)
  └─ setup_billiards_round
  └─ wait_for_billiards_event   ← timeout → balls_stopped 注入
  └─ record_billiards_event × N  (boot_records)
  └─ evaluate_billiards_round
       ├── if_clear    → <terminal_clear>  (pos-0 sacrifice)
       ├── if_fail     → <terminal_fail>   (pos-0 sacrifice)
       └── if_continue → <continue_scene>  (pos-0 sacrifice)

<continue_scene>  (continue: evaluate 経由)
  └─ wait_for_billiards_event  [sacrifice, pos-0 skip]
  └─ wait_for_billiards_event  [actual]
  └─ record_billiards_event × M  (continue_records)
  └─ evaluate_billiards_round (同様の3分岐)
       └── if_continue → <continue_scene>  (自己ループ)

<terminal_clear>  (terminal: evaluate 経由, pos-0 sacrifice)
  └─ end_game(sacrifice)
  └─ end_game

<terminal_fail>   (terminal: evaluate 経由, pos-0 sacrifice)
  └─ end_game(sacrifice)
  └─ end_game
```

---

## T14-2. 入力フィールド仕様

### 2-1. 共通フィールド（前 Generator と同一）

| フィールド | 型 | 必須 | 説明 |
|:---|:---:|:---:|:---|
| `template` | string | ✅ | 固定値 `"event_driven_basic"` |
| `scenario_name` | string | ✅ | YAML ファイル名の一部（英数字・アンダースコアのみ） |
| `description` | string | — | 人間向けコメント（YAML ヘッダに出力） |

### 2-2. ラウンド設定固有フィールド

| フィールド | 型 | 必須 | デフォルト | 説明 |
|:---|:---:|:---:|:---:|:---|
| `setup.shot_limit` | int | ✅ | — | ショット上限（1〜10） |
| `setup.target_count` | int | ✅ | — | クリア必要ポケット数（1〜2） |
| `wait_events` | list(string) | — | `[balls_stopped]` | 待機イベント名のリスト（各要素は有効イベント名） |
| `wait_timeout` | float | — | `0.1` | タイムアウト秒数（headless smoke 用: > 0.0） |
| `boot_records` | list(string) | — | `[]` | boot シーンで記録するイベント名リスト（有効イベント名のみ） |
| `continue_records` | list(string) | — | `[]` | continue シーンで記録するイベント名リスト（有効イベント名のみ） |

**有効イベント名**（`WaitForBilliardsEventTask` / `RecordBilliardsEventTask` 共通）:  
`shot_committed`, `ball_pocketed`, `cue_ball_pocketed`, `balls_stopped`

### 2-3. シーン名フィールド

| フィールド | 型 | 必須 | 制約 | 説明 |
|:---|:---:|:---:|:---|:---|
| `scenes.continue` | string | — | 予約語禁止・英数字アンダースコア | continue 遷移先シーン名（デフォルト: `shoot_again`） |
| `scenes.terminal_clear` | string | ✅ | 予約語禁止・`terminal_fail` と異なる | clear 遷移先シーン名 |
| `scenes.terminal_fail` | string | ✅ | 予約語禁止・`terminal_clear` と異なる | fail 遷移先シーン名 |
| `terminal_result.clear` | string | — | 有効 result 値 | デフォルト: `solved` |
| `terminal_result.fail` | string | — | 有効 result 値 | デフォルト: `failed` |

**予約済み内部シーン名**（`scenes.*` に使用不可）:  
`setup_round`（boot シーン名は常に `setup_round` 固定）

---

## T14-3. バリデーション規則

| 規則 ID | 対象 | 内容 |
|:---|:---|:---|
| V-ED-01 | `template` | `"event_driven_basic"` と完全一致すること |
| V-ED-02 | `scenario_name` | `[a-z0-9_]` のみ。空文字・スペース禁止 |
| V-ED-03 | `setup.shot_limit` | 1〜10 の整数 |
| V-ED-04 | `setup.target_count` | 1〜2 の整数 |
| V-ED-05 | `wait_events` | 非空リスト。各要素は `shot_committed/ball_pocketed/cue_ball_pocketed/balls_stopped` のいずれか |
| V-ED-06 | `wait_timeout` | 浮動小数 > 0.0 |
| V-ED-07 | `boot_records` | 各要素が有効イベント名であること |
| V-ED-08 | `continue_records` | 各要素が有効イベント名であること |
| V-ED-09 | `scenes.terminal_clear/fail` | 両方指定必須。互いに異なること。空文字禁止 |
| V-ED-10 | `scenes.terminal_*` / `scenes.continue` | 予約語 `setup_round` は使用不可 |
| V-ED-11 | `scenes.continue` | `terminal_clear` / `terminal_fail` と異なること |
| V-ED-12 | `terminal_result.clear/fail` | 省略可。指定時は `solved/wrong/failed/lost/timeout` のいずれか |

---

## T14-4. Spec → YAML マッピング規則

### 4-1. boot シーン（`setup_round` — pos-0 sacrifice 不要）

```yaml
setup_round:
  on_enter:
    - action: setup_billiards_round
      payload:
        shot_limit:   <setup.shot_limit>
        target_count: <setup.target_count>

    - action: wait_for_billiards_event
      payload:
        events:  <wait_events>
        timeout: <wait_timeout>

    # boot_records の各要素を順に出力（空の場合はなし）
    - action: record_billiards_event
      payload:
        event: <boot_records[i]>

    - action: evaluate_billiards_round
      payload:
        if_clear:    <terminal_clear>
        if_fail:     <terminal_fail>
        if_continue: <scenes.continue>
```

> **R-1 準拠（確定）**: `boot` シーンは `_ready()` 経由で `load_scene_by_id(start_id)` → `start_actions(index=0)` → `_process()` で index=0 から実行。pos-0 sacrifice **不要**。

### 4-2. continue シーン（pos-0 sacrifice 必要）

`evaluate_billiards_round::execute()` → `load_scene_by_id(continue_scene)` → `start_actions(index=0)` → `execute()` が Success → `step_actions()` で `index++` → pos-0 スキップ。

```yaml
<scenes.continue>:
  on_enter:
    - action: wait_for_billiards_event  # pos-0 sacrifice（スキップされる）
      payload:
        events:  <wait_events>
        timeout: <wait_timeout>
    - action: wait_for_billiards_event  # 実際に実行される
      payload:
        events:  <wait_events>
        timeout: <wait_timeout>

    # continue_records の各要素を順に出力（空の場合はなし）
    - action: record_billiards_event
      payload:
        event: <continue_records[i]>

    - action: evaluate_billiards_round
      payload:
        if_clear:    <terminal_clear>
        if_fail:     <terminal_fail>
        if_continue: <scenes.continue>
```

### 4-3. terminal シーン（clear / fail — pos-0 sacrifice 必要）

```yaml
<terminal_clear>:
  on_enter:
    - action: end_game          # pos-0 sacrifice（スキップされる）
      payload: {result: <terminal_result.clear>}
    - action: end_game          # 実際に実行される
      payload: {result: <terminal_result.clear>}

<terminal_fail>:
  on_enter:
    - action: end_game          # pos-0 sacrifice（スキップされる）
      payload: {result: <terminal_result.fail>}
    - action: end_game          # 実際に実行される
      payload: {result: <terminal_result.fail>}
```

> **R-10 確定（headless HG-4 smoke 済み）**: terminal シーンの pos-0 skip は T13/R-5 と同じメカニズム。  
> `billiards_r8_waiting_smoke.yaml` の `victory` Action count: 2.0 → pos-0 skip → pos-1 `end_game(solved)` 実行確認。

---

## T14-5. Time/Clock Basic との差異

| 観点 | Time/Clock Basic (T13) | Event-Driven Basic (T14) |
|:---|:---|:---|
| 時間管理 | `KernelClock.advance(ms)` による離散 step | `KernelClock.now()` による実時間タイムアウト |
| フレーム依存 | なし（同期的 Task 実行） | あり（`TaskResult::Waiting` が `_process()` フレームを消費） |
| Fake 入力方式 | `load_fake_tap` → 即時 WorldState 書き込み | `wait_for_billiards_event` timeout → `balls_stopped` 自動注入 |
| ループ構造 | 4 中間シーン（advance/judge/resolve/loop_gate） | 1 continue シーン（自己ループ） |
| pos-0 sacrifice | boot 不要 / 全中間シーン + terminal 必要 | boot 不要 / continue + terminal 必要 |
| 結果分岐 | 3 経路（clear/fail/continue）× 4 中間シーン | 3 経路（clear/fail/continue）× 1 continue シーン |
| Task 数 / boot | 2（setup + evaluate） | 3+N（setup + wait + records[N] + evaluate） |
| headless 実行時間 | 即時（全 Task が同フレーム完了） | `wait_timeout × loop_count` 秒（フレーム待機必要） |

---

## T14-6. リスク解決状況

| # | リスク | 状態 | 結果 |
|:---:|:---|:---:|:---|
| R-8 | `TaskResult::Waiting` が headless で正常動作するか。`_process(delta)` → `clock->advance(delta)` が正しく呼ばれ、`timeout: 0.1` 後に `balls_stopped` が注入されるか | ✅ **確定** | 正常動作。`[WaitForBilliardsEventTask] Timeout: injecting balls_stopped.` → evaluate → `solved` を確認。smoke YAML: `billiards_r8_waiting_smoke.yaml` |
| R-9 | continue シーン（`shoot_again`）の pos-0 `wait_for_billiards_event` に sacrifice パターンを適用した場合、2つ目の wait が正常動作するか | ✅ **確定** | pos-0 sacrifice スキップ → pos-1 の actual wait が `Waiting` ポーリング → timeout → `balls_stopped` 注入 → `evaluate Clear → victory → solved` を確認。smoke YAML: `billiards_r9_continue_smoke.yaml` |
| R-10 | terminal（`victory`/`defeat`）の pos-0 `end_game` に sacrifice を適用した場合、pos-1 の `end_game` が正常実行されるか（`billiards_test_completion.md` では未確認扱い） | ✅ **確定** | `victory` Action count: 2.0 → pos-0 skip → pos-1 `end_game(solved)` 実行確認。R-8 smoke の terminal で兼用確認。 |

また、`scenario_runner.cpp` のコード解析により以下の動作が確定した:
- `setup_round`（boot シーン）は `_ready()` 経由の `load_scene_by_id` → `start_actions(index=0)` → `_process()` で index=0 から実行。pos-0 sacrifice **不要**。
- `shoot_again`（continue シーン）は `evaluate_billiards_round::execute()` → `load_scene_by_id()` → `execute()` returns Success → `step_actions()` while ループ `pending_action_index_++` → pos-0 スキップ。**sacrifice 必要**。
- terminal（`victory`/`defeat`）は continue と同じメカニズム。**sacrifice 必要**。

---

## T14-7. 最小 Spec 例

```yaml
template: event_driven_basic
scenario_name: billiards_clear

setup:
  shot_limit:   2
  target_count: 1

boot_records:
  - shot_committed
  - ball_pocketed

scenes:
  terminal_clear: victory
  terminal_fail:  defeat
```

### 期待出力 YAML 骨格（抜粋）

```yaml
start_scene: setup_round

scenes:
  setup_round:
    on_enter:
      - action: setup_billiards_round
        payload:
          shot_limit:   2
          target_count: 1

      - action: wait_for_billiards_event
        payload:
          events:  [balls_stopped]
          timeout: 0.1

      - action: record_billiards_event
        payload:
          event: shot_committed

      - action: record_billiards_event
        payload:
          event: ball_pocketed

      - action: evaluate_billiards_round
        payload:
          if_clear:    victory
          if_fail:     defeat
          if_continue: shoot_again

  shoot_again:
    on_enter:
      - action: wait_for_billiards_event  # pos-0 sacrifice
        payload:
          events:  [balls_stopped]
          timeout: 0.1
      - action: wait_for_billiards_event
        payload:
          events:  [balls_stopped]
          timeout: 0.1
      - action: evaluate_billiards_round
        payload:
          if_clear:    victory
          if_fail:     defeat
          if_continue: shoot_again

  victory:
    on_enter:
      - action: end_game              # pos-0 sacrifice
        payload: {result: solved}
      - action: end_game
        payload: {result: solved}

  defeat:
    on_enter:
      - action: end_game              # pos-0 sacrifice
        payload: {result: failed}
      - action: end_game
        payload: {result: failed}
```

---

## T14-8. 実装予定ファイル

| ファイル | 役割 |
|:---|:---|
| `tools/gen_scenario_event_driven.py` | Event-Driven Basic Scaffold Generator 本体 |
| `scenarios/generated/event_driven_basic_expected_output.yaml` | 期待出力サンプル（T14 実装後に追加） |
| `docs/t14_gen_event_driven_completion.md` | T14 完了メモ（T14 実装後に作成） |

> **実装制約**: この節（T14-1〜T14-8）は設計確定済み。  
> **R-8 / R-9 / R-10 はいずれも headless HG-4 smoke で確認済み**（2026-04-01）。Generator 本体の実装に進める。
