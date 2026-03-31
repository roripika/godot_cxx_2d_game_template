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
