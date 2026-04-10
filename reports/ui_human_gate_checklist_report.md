# Summary
- `docs/ui_human_gate_checklist.md` を新規作成し、IB-03（UI line human gate checklist）を実務運用できる形で定義した。
- `human_gate_asset_policy.md` の UI 観点を AG-1〜AG-4 の具体チェック項目、判定目安、証跡テンプレートに展開した。
- `docs/integration_backlog.md` の IB-03 を `Done` に更新し、次段を IB-04 以降へ進める状態にした。

## Files Changed
- `docs/ui_human_gate_checklist.md`: UI 向け Gate 判定チェックリストの新規追加
- `docs/integration_backlog.md`: IB-03 の status を `Done` に更新、次アクションを更新
- `docs/README.md`: `ui_human_gate_checklist.md` への最小導線を追加
- `reports/ui_human_gate_checklist_report.md`: 本タスクの実施記録を追加

## 変更理由
- IB-03 の deliverable を作成し、IB-02 までで固定した契約を Human Gate 実運用へ接続するため。
- UI line と主軸 repo の責務分離を維持しつつ、主軸 repo 側で残すべき証跡を最小形で固定するため。

## 変更しなかった候補
- 候補A: runtime 実装（input handling / layout 自動調整）を checklist 対象に含める。
  - 不採用理由: 本タスクは UI asset 判定に限定する契約であり、責務境界を越えるため。
- 候補B: UI line 内部ログや比較画像の保存規約を主軸 repo 側で詳細定義する。
  - 不採用理由: 主軸 repo に持ち込まない方針と衝突するため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- UI 向け AG-1〜AG-4 の具体チェック項目と PASS / REWORK / STOP の目安を固定した。
- 主軸 repo に残す証跡項目と最小 YAML テンプレートを追加し、IB-04 へ進める前提を満たした。

## Risks / Open Issues
- 最初の統合対象画面・asset_id が未確定のため、AG-1 の適用時に解像度前提や variant の具体値が揺れる可能性がある。
- UI line 側の実出力形式差異がある場合、AG-2 の形式チェック観点を IB-04 で微調整する必要がある。

## 次に着手すべき1タスク
- `IB-04 UI line first integration scenario` を起票し、最初の接続対象を 1 件に固定する。
