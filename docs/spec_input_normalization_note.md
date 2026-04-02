# Spec Input Normalization Note

**ステータス**: v1.0（方針メモ）  
**作成日**: 2026-04-02  
**対象**: 4 generator の Structured Spec 入力

---

## 1. 目的

template ごとに揺れている Structured Spec 入力キーを整理し、  
Phase 4 での共通化・回帰強化に耐える入力インターフェース方針を定義する。

---

## 2. 現状の揺れ（要約）

4 generator で共通しているのは `template` と `scenario_name` のみで、  
終端シーン指定・setup 情報・補助情報の配置が template ごとに異なる。

主な揺れ:
- terminal 指定: `scenes.terminal_*` と top-level `terminal_*` が混在
- setup 指定: `setup.*`（event_driven）と top-level（turn_grid/time_clock）が混在
- 補助記述: `description` や `goal` の位置が統一されていない

---

## 3. 正規化方針（Canonical v2）

### 3-1. 共通コアキー（全 template 共通）

```yaml
template: <template_name>
scenario_name: <snake_case_name>
meta:
  description: <optional string>
scenes:
  start: <optional>              # template で意味がある場合のみ
  continue: <optional>           # template で意味がある場合のみ
  terminal_clear: <required>
  terminal_fail: <required>
terminal_result:
  clear: <optional>
  fail: <optional>
```

### 3-2. template 固有は `spec.*` に集約

```yaml
spec:
  # template ごとの固有フィールド
```

---

## 4. template 別マッピング

| template | 現行キー | 正規化キー（提案） |
|:---|:---|:---|
| `branching_basic` | `branches`, `check_type`, `goal.*`, `scenes.*` | `spec.branches`, `spec.check_type`, `spec.goal.*`, `scenes.*` |
| `turn_grid_basic` | `player`, `enemies`, `first_command`, `terminal_*` | `spec.player`, `spec.enemies`, `spec.first_command`, `scenes.terminal_*` |
| `time_clock_basic` | `notes`, `taps`, `advance_ms`, `clear_hit_count`, `max_miss_count`, `scenes.*` | `spec.notes`, `spec.taps`, `spec.advance_ms`, `spec.clear_hit_count`, `spec.max_miss_count`, `scenes.*` |
| `event_driven_basic` | `setup.*`, `wait_events`, `wait_timeout`, `boot_records`, `continue_records`, `scenes.*` | `spec.setup.*`, `spec.wait_events`, `spec.wait_timeout`, `spec.boot_records`, `spec.continue_records`, `scenes.*` |

---

## 5. 互換方針（重要）

- 既存 spec（v1）を即廃止しない
- まずは **read 時に v1/v2 両対応** を許可し、内部表現を Canonical v2 に正規化する
- write（サンプル/新規 fixture）は v2 へ段階移行する
- validator/generator のエラーメッセージは当面 v1 キーも併記する

---

## 6. 段階導入（推奨）

1. Step 1: 正規化関数仕様を文書化（本メモ）
2. Step 2: テスト fixture に v2 サンプルを追加（v1 は残す）
3. Step 3: generator 側で v1/v2 受理 + 内部正規化
4. Step 4: v1 非推奨表示（完全削除は別フェーズ）

---

## 7. 非対象（このメモでやらないこと）

- generator 実装コードの変更
- validator ロジックの変更
- `src/core` 変更
- v1 spec の強制廃止

---

## 8. 完了条件（P4-05）

1. 入力揺れが項目別に整理されている
2. Canonical 形式が明示されている
3. 互換方針（v1/v2）と段階導入順が定義されている

---

## 9. 参照

- [`generator_spec_schema.md`](generator_spec_schema.md)
- [`generator_commonization_candidates.md`](generator_commonization_candidates.md)
- [`generator_regression_expansion_plan.md`](generator_regression_expansion_plan.md)
- [`phase4_backlog.md`](phase4_backlog.md)

