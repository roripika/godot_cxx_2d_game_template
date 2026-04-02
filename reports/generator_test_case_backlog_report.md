# generator_test_case_backlog Report

## Summary
- `docs/generator_test_case_backlog.md` を新規作成し、回帰テスト候補をケース台帳化した。
- ケースを Batch A/B/C の優先順に整理し、各ケースに期待結果と除外条件を付与した。
- `docs/phase4_backlog.md` の P4-07 を Done に更新し、P4-08 を Ready に変更した。
- `docs/README.md` に台帳への導線を追加した。

## Files Changed
- `docs/generator_test_case_backlog.md`: 新規作成。
- `docs/phase4_backlog.md`: P4-07 を Done、P4-08 を Ready に更新。
- `docs/README.md`: 参照リンクを追加。
- `reports/generator_test_case_backlog_report.md`: 本レポートを新規作成。

## 変更理由
- P4-04 は計画レベルだったため、実装に着手できる粒度（ケース台帳）へ落とし込む必要があった。
- 「期待結果」と「除外条件」を各ケースに明示して、運用時の誤判定を減らすため。

## 変更しなかった候補
- 候補A: 直接 `tests/test_generators.py` を Batch A まで実装する  
  理由: P4-07 はケース台帳作成タスクであり、実装は次タスクに分離するため。
- 候補B: 除外条件をグローバルルールのみで管理する  
  理由: ケース単位で除外理由が異なるため、個別定義を優先した。

## Tests Run
- `nl -ba docs/generator_test_case_backlog.md | sed -n '1,260p'`: pass
- `nl -ba docs/phase4_backlog.md | sed -n '34,60p'`: pass

## Result
- P4-07 の完了条件（優先順・期待結果・除外条件の定義）を満たした。

## Risks / Open Issues
- P4-08 で中間レビューを作る際、Batch A 未実装のため「計画完了」と「実装完了」を明確に分けて記録する必要がある。

## 次に着手すべき1タスク
- `P4-08 phase4_midpoint_review` を作成し、P0/P1 完了内容と P2 残タスクを整理する。

