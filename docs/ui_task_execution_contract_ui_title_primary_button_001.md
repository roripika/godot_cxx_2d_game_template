# UI TASK EXECUTION CONTRACT

**ステータス**: draft v0.1  
**作成日**: 2026-04-10  
**契約種別**: first executable contract

---

## 0. Task Name

`ui_title_primary_button_001_first_execution_contract`

## 1. Objective

`ui/title/primary_button_001` を生成し、title screen の main action button として
統合判定可能な状態（成果物・Gate 証跡・判定記録あり）にする。

## 2. Background

- `ui_first_integration_scenario.md` で最初の統合対象は `ui/title/primary_button_001` に固定済み。
- `ui_integration_contract.md` で UI line の入出力契約（`svg` / `png` / metadata）が定義済み。
- `ui_human_gate_checklist.md` で AG-1〜AG-4 の判定項目と証跡形式が定義済み。
- 主軸 repo は契約・判定・統合記録を保持し、UI line 内部実装は保持しない。

## 3. Scope

- 対象 `asset_id` の UI spec 準備または更新
- UI line 実行（対象 1 asset のみ）
- 生成物 `svg` / `png` / metadata の取得確認
- AG-1〜AG-4 の判定材料整理
- 主軸 repo 側 report の作成

## 4. Out of Scope

- `src/` 配下の変更
- ゲーム本体コード変更
- UI compiler / renderer の内部改修
- runtime layout / input handling 実装
- 複数 `asset_id` の同時導入
- タイトル画面全体の設計変更

## 5. Hard Constraints

- 1 task 1 asset を厳守する
- 対象は `ui/title/primary_button_001` のみ
- `ui_kind` / `usage_scope` / `integration_target` を必須記録する
- Gate 結果は AG-1〜AG-4 をすべて残す
- 判定は `PASS` / `REWORK` / `STOP` のいずれかを明示する
- 主軸 repo に UI line 内部詳細（実験ログ全文等）を持ち込まない

## 6. Fixed Contract Values

- `asset_id`: `ui/title/primary_button_001`
- `ui_kind`: `button`
- `usage_scope`: `title_screen_primary_action`
- `integration_target`: `title_screen/main_action_button`
- expected outputs: `svg`, `png`, `metadata`

## 7. Inputs

- `docs/asset_contracts.md`
- `docs/ui_pipeline_responsibility.md`
- `docs/ui_integration_contract.md`
- `docs/ui_human_gate_checklist.md`
- `docs/ui_first_integration_scenario.md`
- 対象 UI spec file（UI line repo 側）

## 8. Expected Deliverables

- `ui/title/primary_button_001` の `svg`
- `ui/title/primary_button_001` の `png`
- 対象 asset の metadata（最低: `asset_id`, `asset_type`, `status`, `usage_scope`, `source_spec`）
- Gate 判定付き実行 report（主軸 repo 側）

## 9. Work Steps

1. 参照文書を確認し固定値を再確認する
2. UI line 用 spec を準備し対象 `asset_id` を 1 件に固定する
3. UI line を実行し `svg` / `png` / metadata を取得する
4. AG-1〜AG-4 の判定材料を収集する
5. `PASS` / `REWORK` / `STOP` 判定を Gate ごとに記録する
6. report を作成し、最終 status を確定する

## 10. AG-1〜AG-4 Evidence Requirements

- AG-1: 画面用途、`ui_kind`、`usage_scope`、`integration_target` が明確である証跡
- AG-2: 可読性・余白・形式整合（`svg` / `png` / metadata 揃い）の証跡
- AG-3: 視認性・一貫性・primary button としての強さの証跡
- AG-4: タイトル画面仮配置時の利用可否・違和感有無の証跡

## 11. Decision Policy (`PASS` / `REWORK` / `STOP`)

- `PASS`: 次 Gate へ進行可能。重大不整合なし
- `REWORK`: 方向性は合うが修正が必要。修正後に同一 Gate を再評価
- `STOP`: 要件不明、責務逸脱、用途不整合で継続不可

## 12. Completion Criteria

- 対象 `asset_id` が 1 件に固定されている
- `svg` / `png` / metadata が揃っている
- `ui_kind` / `usage_scope` / `integration_target` が report に記録されている
- AG-1〜AG-4 の判定結果が残っている
- 最終 status（`reviewed` 以上 or `REWORK` / `STOP` 理由付き）が記録されている

## 13. Stop Conditions

- 対象 `asset_id` を 1 件に絞れない
- `svg` / `png` / metadata のいずれかが取得できない
- UI line 側仕様不明で契約項目を満たせない
- UI compiler / renderer 内部改修が前提になっている
- 主軸 repo 側へ runtime 実装変更が必要になる
- AG 判定に必要な証跡が不足し、合理的判定ができない

## 14. Output Format (Report)

```md
# Summary

## Files Changed

## Asset ID
- `ui/title/primary_button_001`

## UI Kind
- `button`

## Usage Scope
- `title_screen_primary_action`

## Integration Target
- `title_screen/main_action_button`

## Outputs
- `svg`: pass/fail
- `png`: pass/fail
- `metadata`: pass/fail

## Gate Results
- AG-1: PASS|REWORK|STOP
- AG-2: PASS|REWORK|STOP
- AG-3: PASS|REWORK|STOP
- AG-4: PASS|REWORK|STOP

## Final Status
- `draft|generated|reviewed|approved|integrated|deprecated`

## Decision
- `PASS|REWORK|STOP`
- Reason:

## Risks / Open Issues

## Suggested Next Step
```

## 15. Execution Policy

- まず読む、次に最小変更で実施する
- 推測で scope を広げない
- 不要な命名変更・広域整形をしない
- 失敗時は `REWORK` / `STOP` の理由を明示する

## 16. Instruction to Executor

上記契約を厳守し、`ui/title/primary_button_001` の実行・確認・記録を行うこと。
禁止範囲に触れる必要が出た場合は作業を中断し、停止理由を report に記載すること。
