# Summary
- `docs/ui_pipeline_responsibility.md` を追加し、IB-01（UI line responsibility note）の責務境界を主軸 repo 観点で固定した。
- 主軸 repo と UI line repo の分離、主軸に残す証跡、持ち込まない情報を明文化した。
- `docs/integration_backlog.md` の IB-01 を Done に更新し、次実行順を IB-02 以降へ進めた。

## Files Changed
- `docs/ui_pipeline_responsibility.md`: UI line 責務境界の定義（対象/対象外、責務分担、完了条件、停止条件）
- `docs/integration_backlog.md`: IB-01 の status を `Done` に更新、次アクションを更新
- `docs/README.md`: `ui_pipeline_responsibility.md` への導線追加

## 変更理由
- integration backlog の IB-01 を実体化し、UI line 先行の統合モデルを抽象から実行段階へ進めるため。
- UI 実装詳細を主軸 repo に持ち込まない境界を先に固定し、後続の IB-02/IB-03 の前提を揃えるため。

## 変更しなかった候補
- 候補A: `ui_integration_contract.md` まで同時作成する。
  - 不採用理由: 1タスク1 deliverable の原則を守るため（IB-02 で扱う）。
- 候補B: UI line repo 側の compiler/renderer 詳細を本文へ含める。
  - 不採用理由: 主軸 repo の責務境界を越えるため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- IB-01 の deliverable が追加され、integration backlog 上で `Done` に進行した。
- 次段（IB-02: UI line integration contract）へ進める前提が文書上で成立した。

## Risks / Open Issues
- UI line repo 側の実際の出力形式が未確定の場合、IB-02 で契約の再調整が必要。
- asset_id 台帳（`asset_id_registry.md`）未作成のため、重複防止ルールは後続タスクで固定が必要。

## 次に着手すべき1タスク
- `IB-02 UI line integration contract` を起票し、UI line の入出力/配置/失敗時扱いを固定する。
