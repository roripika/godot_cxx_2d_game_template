# Summary
- `docs/human_gate_asset_policy.md` を新規作成し、asset type ごとの AG-1〜AG-4 観点差を明文化した。
- PASS / REWORK / STOP の運用基準と、主軸 repo に残す証跡を固定した。
- `asset_pipeline_overview.md` / `asset_contracts.md` / `integration_map.md` と整合するよう、適用範囲と参照関係を明示した。

## Files Changed
- `docs/human_gate_asset_policy.md`: Asset Human Gate 方針（AG-1〜AG-4、判定値、種別別観点、証跡）
- `docs/README.md`: `human_gate_asset_policy.md` への導線追加

## 変更理由
- 既存3文書（overview/contracts/map）では共通枠は定義済みだが、asset type ごとの審査観点差が未整理だったため。
- UI/audio/character/motion/fx/movie の拡張時に、Gate 判定が散らばらない運用基盤を先に作るため。

## 変更しなかった候補
- 候補A: 各ライン固有の詳細チェックリスト（数値閾値、波形解析条件など）まで本ドキュメントに含める。
  - 不採用理由: 主軸 repo の責務を超え、各ライン repo 側の詳細実装領域に入るため。
- 候補B: scenario generator の HG 定義まで同一文書に統合する。
  - 不採用理由: 既存の scenario HG 系文書と責務が異なり、境界を曖昧にするため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- 主軸 repo で Asset Human Gate 運用を定義する4本目の基盤文書が追加され、
  「overview → contracts → integration map → gate policy」の流れが成立した。

## Risks / Open Issues
- 現時点は方針レベルで、Gate 判定の定量化（しきい値・自動チェック項目）は未定義。
- 実運用での証跡フォーマット（テンプレート化）は今後のタスクで固定が必要。

## 次に着手すべき1タスク
- `docs/integration_backlog.md` を追加し、UI line からの Gate 適用タスクを順序付きで管理する。
