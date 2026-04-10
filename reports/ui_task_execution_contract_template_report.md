# Summary
- `docs/ui_task_execution_contract_template.md` を新規作成し、IB-05 の deliverable（UI task 用の再利用可能な実行契約テンプレート）を定義した。
- scope / out of scope / completion criteria / stop conditions / output format / execution policy をテンプレート内に固定し、契約駆動投入の型を明文化した。
- `ui_first_integration_scenario.md` と整合する最初の適用例として `ui/title/primary_button_001` を明記した。

## Files Changed
- `docs/ui_task_execution_contract_template.md`: UI task 実行契約テンプレートを新規追加
- `docs/integration_backlog.md`: IB-05 の status を `Done` に更新、次アクションを更新
- `docs/README.md`: `ui_task_execution_contract_template.md` への最小導線を追加
- `reports/ui_task_execution_contract_template_report.md`: 本タスクの実施内容を記録

## 変更理由
- IB-05 の目的である「UI line task を同じ型で起票する」ため、契約項目をテンプレート化して再利用可能にする必要があるため。
- 主軸 repo の責務境界を維持しつつ、Human Gate 証跡と integration target の追跡を task 単位で固定するため。

## 変更しなかった候補
- 候補A: UI line の compiler / renderer 内部改修手順までテンプレートに含める。
  - 不採用理由: 主軸 repo の責務境界を越えるため。
- 候補B: 複数 asset を同時投入するテンプレートに拡張する。
  - 不採用理由: 1 task 1 asset の運用原則に反し、初期運用での切り分けが難しくなるため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- IB-05 の完了条件（再利用可能な契約テンプレートの固定）を満たした。
- 次段として、`ui/title/primary_button_001` を対象に最初の実タスク契約を起票できる状態になった。

## Risks / Open Issues
- UI line 側で実際に要求される spec 項目が追加された場合、テンプレートの Inputs / Completion Criteria の追補が必要。
- `asset_id_registry.md` が未整備のまま運用が進む場合、将来的に命名重複管理の負荷が上がる可能性がある。

## 次に着手すべき1タスク
- `ui/title/primary_button_001` を対象に、`UI TASK EXECUTION CONTRACT` を実際に起票する。
