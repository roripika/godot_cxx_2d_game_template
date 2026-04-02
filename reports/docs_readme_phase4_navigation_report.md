# docs_readme_phase4_navigation Report

## Summary
- `docs/README.md` に「Phase 4 最短導線」セクションを追加した。
- Phase 4 着手時に読むべき 4 文書を固定順で明示した。
- `docs/phase4_backlog.md` の P4-03 を Done に更新した。
- 直近実行順を未完了タスク（P4-04〜P4-07）へ更新した。

## Files Changed
- `docs/README.md`: Phase 4 最短導線セクションを追加。
- `docs/phase4_backlog.md`: P4-03 を Done 化し、直近実行順を更新。
- `reports/docs_readme_phase4_navigation_report.md`: 本レポートを新規作成。

## 変更理由
- 既存 README は Phase 4 文書リンクが長い一覧の中に埋もれており、着手順が即時判断しにくかったため。
- Backlog の完了状態に合わせて、次の実行順を未完了タスク中心に更新する必要があったため。

## 変更しなかった候補
- 候補A: README 全体を Phase 別に全面再編する  
  理由: 変更範囲が大きく、P4-03 の「導線整理」範囲を超えるため見送った。
- 候補B: Phase 4 専用 README を別ファイルで新設する  
  理由: まず既存 `docs/README.md` から最短導線を提供する方が運用コストが低いため。

## Tests Run
- `nl -ba docs/README.md | sed -n '1,80p'`: pass
- `nl -ba docs/phase4_backlog.md | sed -n '34,60p'`: pass

## Result
- P4-03 の完了条件（Phase 4 関連 docs への最短導線追加）を満たした。

## Risks / Open Issues
- README の Phase 3 項目は依然として長く、将来の情報追加で再び導線が埋もれる可能性がある。

## 次に着手すべき1タスク
- `P4-04 generator_regression_expansion_plan` を起票して回帰強化計画を文書化する。

