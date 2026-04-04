# Summary
- `docs/integration_backlog.md` を UI line 先行の順序付きタスク（IB-01〜IB-06）形式へ更新した。
- 各タスクに deliverable と completion criteria を明示し、抽象方針を実行計画へ接続した。
- 既存の asset 文書群（overview/contracts/map/gate）との整合を保ち、主軸 repo の統合順管理として利用できる状態にした。

## Files Changed
- `docs/integration_backlog.md`: UI line 先行の順序付き backlog へ更新
- `docs/README.md`: `integration_backlog.md` 導線（既存）を維持
- `docs/integration_map.md`: 必要ドキュメント欄の重複行を整理
- `reports/integration_backlog_report.md`: 本更新内容の記録

## 変更理由
- 次段を「抽象方針」から「実行タスク起票」へ進めるため。
- UI line を最初の連携モデルとして固定し、以降の展開順を明確化するため。

## 変更しなかった候補
- 候補A: IB-01〜IB-06 を同時に実体文書まで作成する。
  - 不採用理由: 1タスク1目的の原則に反し、差分が過大になるため。
- 候補B: integration backlog を `integration_map.md` に統合する。
  - 不採用理由: 接続俯瞰と実行管理の責務を分離したほうが運用しやすいため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- `integration_backlog.md` が UI line 先行で実行可能な形に整備され、次タスク（IB-01）を即時起票できる状態になった。

## Risks / Open Issues
- IB-01 起票時に、UI line 側 repo の責務境界が不明瞭だと再調整が必要。
- `asset_id_registry.md` 未作成のため、ID 重複防止ルールは後続タスクで固定する必要がある。

## 次に着手すべき1タスク
- `IB-01 UI line responsibility note` を起票し、主軸 repo と UI line repo の責務境界を固定する。
