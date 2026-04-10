# Summary
- `docs/ui_task_execution_contract_ui_title_primary_button_001.md` を追加し、`ui/title/primary_button_001` 向けの最初の実 task 契約を固定した。
- 契約内で `asset_id` / `ui_kind` / `usage_scope` / `integration_target` / 出力物（svg/png/metadata）を明示した。
- AG-1〜AG-4 の証跡要求、`PASS` / `REWORK` / `STOP` 判定方針、Stop Conditions、report 形式を実行可能な形で定義した。

## Files Changed
- `docs/ui_task_execution_contract_ui_title_primary_button_001.md`: 最初の UI 実 task 契約を新規追加
- `reports/ui_task_execution_contract_ui_title_primary_button_001_report.md`: 本タスクの記録を追加

## 変更理由
- UI line 側の方針文書（IB-01〜IB-06）を実運用へ接続するため、最初の実 task 契約を固定する必要があるため。
- 対象を 1 asset に限定し、判定・停止条件・証跡フォーマットを先に固定して実行時のぶれを抑えるため。

## 変更しなかった候補
- 候補A: 複数 UI asset を同時対象にした契約を作る。
  - 不採用理由: 1 task 1 asset 原則に反し、失敗時の切り分けが難しくなるため。
- 候補B: UI line 内部実装詳細（renderer/engine）まで契約へ含める。
  - 不採用理由: 主軸 repo の責務境界を超えるため。

## Tests Run
- `not run`（docs/report のみ変更）

## Result
- `ui/title/primary_button_001` の実行契約が起票され、最初の UI 実タスクへ着手可能な状態になった。

## Risks / Open Issues
- UI line repo 側の実 spec 形式と契約項目に差異がある場合、Inputs/Completion Criteria の軽微調整が必要。
- 実行時に AG-4 判定用の仮配置証跡が不足する可能性があるため、先に取得手順の合意が必要。

## 次に着手すべき1タスク
- 本契約に基づき、`ui/title/primary_button_001` の UI line 実行と Gate 判定レポート作成を実施する。
