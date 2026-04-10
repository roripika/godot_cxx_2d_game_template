# Summary
- `docs/ui_integration_contract.md` を新規作成し、IB-02（UI line integration contract）の入出力・契約項目を固定した。
- `asset_contracts.md` / `integration_map.md` / `human_gate_asset_policy.md` / `ui_pipeline_responsibility.md` と整合するよう、`asset_id`・metadata・status・Human Gate 接続・失敗時契約を明記した。
- `docs/integration_backlog.md` の IB-02 を `Done` に更新し、次段を IB-03/IB-04/IB-05 に進める形へ調整した。

## Files Changed
- `docs/ui_integration_contract.md`: IB-02 の本体文書を追加（入力、出力、asset_id、metadata、status、配置、Human Gate、失敗時契約）
- `docs/integration_backlog.md`: IB-02 の status を `Done` に更新、次アクションを更新
- `docs/README.md`: `ui_integration_contract.md` への最小導線を追加
- `reports/ui_integration_contract_report.md`: 本タスクの実施内容を記録

## 変更理由
- IB-02 の deliverable を実体化し、IB-01 で固定した責務境界を実運用可能な入出力契約へ接続するため。
- UI line の内部実装を主軸 repo に持ち込まずに統合判断できるよう、契約と記録項目だけを主軸側で保持する方針を具体化するため。

## 変更しなかった候補
- 候補A: UI line の compiler / renderer 詳細仕様を本契約に含める。
  - 不採用理由: 主軸 repo の責務境界を越えるため。
- 候補B: UI animation / input handling / runtime layout 自動調整を同時に契約化する。
  - 不採用理由: IB-02 のスコープ（静的 UI asset 契約）を超えるため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- IB-02 の完了条件（入出力、asset_id、metadata、status、配置、Human Gate 接続、失敗時契約の明文化）を満たした。
- backlog 上の UI line P0 は IB-03（checklist）へ進行可能になった。

## Risks / Open Issues
- `integration_target` の命名辞書は未固定のため、IB-04 で最初の対象画面を 1 件に絞る際に具体値を確定する必要がある。
- UI line 側の実出力フォーマット差異（例: `pdf` 非対応）がある場合、実運用で契約値の再調整が必要。

## 次に着手すべき1タスク
- `IB-03 UI line human gate checklist` を起票し、AG-1〜AG-4 を UI 実運用チェック項目へ具体化する。
