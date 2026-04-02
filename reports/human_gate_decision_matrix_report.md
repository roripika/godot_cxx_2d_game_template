# human_gate_decision_matrix Report

## Summary
- `docs/human_gate_decision_matrix.md` を新規作成し、HG-1〜HG-4 の判定条件を表形式で固定した。
- 各 Gate に `PASS / REWORK / STOP` を定義し、判定の一貫性を持たせた。
- template 別の HG-2 重点確認項目（branching/turn_grid/time_clock/event_driven）を明文化した。
- `docs/README.md` に導線を追加し、参照しやすくした。

## Files Changed
- `docs/human_gate_decision_matrix.md`: 新規作成。
- `docs/README.md`: 参照リンクを追加。
- `docs/phase4_backlog.md`: P4-02 の Status を Done に更新。
- `reports/human_gate_decision_matrix_report.md`: 本レポートを新規作成。

## 変更理由
- Human Gate を維持する方針は既に定義されていたが、判定実務の粒度が文書間で分散していたため。
- Gate ごとに停止条件と証跡を固定し、運用時の迷いを減らすため。

## 変更しなかった候補
- 候補A: HG 判定を generator 実装側へ組み込む  
  理由: Human Gate の通過判定は人間が持つべきであり、設計方針に反する。
- 候補B: 各テンプレート専用の HG 文書を個別作成する  
  理由: まず共通判定表を1本化し、差分は HG-2 重点欄で吸収する方が保守しやすい。

## Tests Run
- `rg -n "HG-1|HG-2|HG-3|HG-4|PASS|REWORK|STOP" docs/human_gate_decision_matrix.md`: pass
- `git diff -- docs/human_gate_decision_matrix.md docs/phase4_backlog.md docs/README.md`: pass

## Result
- Human Gate 判定表が追加され、P4-02 の完了条件を満たした。

## Risks / Open Issues
- HG-4 の runtime 経路確認粒度（どこまでログ化するか）は今後のテスト運用で追加調整が必要。

## 次に着手すべき1タスク
- `P4-03 docs_readme_phase4_navigation`（Phase 4 docs 導線の最短化を完了させる）。

