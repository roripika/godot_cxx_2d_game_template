# generated_lifecycle_check_sync Report

## Summary
- `docs/generated_dir_policy.md` を v1.1 に更新し、HG-2/HG-3/HG-4 のライフサイクル記述を現運用へ同期した。
- ライフサイクルに `PASS / REWORK / STOP` を明示し、Gate ごとの遷移条件を統一した。
- expected output サンプル一覧を 4 テンプレート分に更新した。
- `docs/phase4_backlog.md` の P4-06 を Done に更新した。

## Files Changed
- `docs/generated_dir_policy.md`: HG ライフサイクル同期・期待出力一覧更新。
- `docs/phase4_backlog.md`: P4-06 を Done 化、直近実行順を更新。
- `reports/generated_lifecycle_check_sync_report.md`: 本レポートを新規作成。

## 変更理由
- 既存運用は HG-2/HG-3/HG-4 の実行手順は記述されていたが、判定ステータス（PASS/REWORK/STOP）が未明文化で、判断粒度が文書間でずれていたため。
- expected output サンプルの記載が branching のみで、現状の 4 テンプレート運用と不一致だったため。

## 変更しなかった候補
- 候補A: `generated_dir_policy.md` を全面再構成して章立てを刷新する  
  理由: P4-06 は「同期」が目的であり、最小差分で整合性を取る方針のため。
- 候補B: `README.md` に generated 運用の詳細を移植する  
  理由: 運用詳細は `generated_dir_policy.md` に集約し、README は導線のみ維持するため。

## Tests Run
- `nl -ba docs/generated_dir_policy.md | sed -n '1,260p'`: pass
- `nl -ba docs/phase4_backlog.md | sed -n '34,58p'`: pass

## Result
- P4-06 の完了条件（HG-2/HG-3/HG-4 ライフサイクル記述の同期）を満たした。

## Risks / Open Issues
- 実運用で `STOP` 判定が発生した際の通知テンプレートは未整備のため、次タスクでフォーマット化すると運用が安定する。

## 次に着手すべき1タスク
- `P4-07 generator_test_case_backlog` を作成し、回帰ケースの詳細バックログを固定する。

