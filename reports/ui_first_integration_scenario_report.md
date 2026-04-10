# Summary
- `docs/ui_first_integration_scenario.md` を新規作成し、IB-04 の要求どおり最初の UI integration 対象を 1 件に固定した。
- 候補比較（A/B/C）を明記し、最終的に `A: タイトル画面の primary button` を採用理由付きで確定した。
- `asset_id` / `usage_scope` / `integration_target` / Human Gate 適用方針 / 成功条件 / 保留条件を定義し、次段の IB-05 へ接続した。

## Files Changed
- `docs/ui_first_integration_scenario.md`: IB-04 本体文書を追加（候補比較、採用理由、想定 asset、Gate 運用、成功条件）
- `docs/integration_backlog.md`: IB-04 の status を `Done` に更新、次アクションを更新
- `docs/README.md`: `ui_first_integration_scenario.md` への最小導線を追加
- `reports/ui_first_integration_scenario_report.md`: 本タスクの実施内容を記録

## 変更理由
- UI line 連携の初回ケースを 1 件へ固定しないと、後続 task（契約テンプレート、実運用 task）の前提が揺れるため。
- IB-01〜IB-03 で固定した責務・契約・Gate の文書を、実シナリオへ接続する必要があるため。

## 変更しなかった候補
- 候補A: 結果画面の報酬パネルを初回対象にする。
  - 不採用理由: 情報量が多く、最小成功例としては重いため。
- 候補B: ダイアログ UI 最小セットを初回対象にする。
  - 不採用理由: 複数 asset が絡み、失敗時の切り分けが難しくなるため。

## Tests Run
- `not run`（docs のみ変更）

## Result
- 初回統合対象が `ui/title/primary_button_001` に固定され、IB-04 の完了条件を満たした。
- 次段の IB-05（UI line task execution contract template）へ進める前提が整った。

## Risks / Open Issues
- タイトル画面の最終ラベル（`START` / `GAME START` / `はじめる`）は未確定のため、個別 task 契約で確定が必要。
- UI line 側の spec 形式との差異がある場合、`integration_target` と metadata の項目名を調整する可能性がある。

## 次に着手すべき1タスク
- `IB-05 UI line task execution contract template` を起票し、`ui/title/primary_button_001` 向けの task 契約投入形式を固定する。
