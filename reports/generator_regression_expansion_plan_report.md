# generator_regression_expansion_plan Report

## Summary
- `docs/generator_regression_expansion_plan.md` を新規作成し、generator 回帰強化の追加ケースと導入順を定義した。
- ケースを ID（RGE-01〜RGE-09）で整理し、P0/P1/P2 の優先度を付与した。
- `docs/phase4_backlog.md` の P4-04 を Done に更新した。
- `docs/README.md` に計画書への導線を追加した。

## Files Changed
- `docs/generator_regression_expansion_plan.md`: 新規作成。
- `docs/phase4_backlog.md`: P4-04 を Done 化、直近実行順を更新。
- `docs/README.md`: 参照リンクを追加。
- `reports/generator_regression_expansion_plan_report.md`: 本レポートを新規作成。

## 変更理由
- 現行テストは正常系の退行検知に寄っており、異常系の検知を計画的に追加する必要があった。
- いきなり実装拡張すると差分が広がるため、先にケース境界と導入順を固定した。

## 変更しなかった候補
- 候補A: 直接 `tests/test_generators.py` を拡張して異常系を同時実装する  
  理由: P4-04 は計画タスクであり、実装は次タスクへ分離する方針のため。
- 候補B: generator 本体にテスト用フラグを追加して失敗系を注入する  
  理由: `src/core` 非変更方針に加え、generator 本体改修を伴うため今回スコープ外。

## Tests Run
- `nl -ba docs/generator_regression_expansion_plan.md | sed -n '1,240p'`: pass
- `nl -ba docs/phase4_backlog.md | sed -n '34,62p'`: pass

## Result
- P4-04 の完了条件（追加すべき回帰ケースと実行順の定義）を満たした。

## Risks / Open Issues
- RGE-09（expected 差分検出）はテスト用 fixture の設計を誤ると誤検知が増える可能性がある。
- P4-05/P4-06 の内容次第で、回帰ケース ID の一部再整理が必要になる可能性がある。

## 次に着手すべき1タスク
- `P4-05 spec_input_normalization_note` を起票して入力揺れの統一方針を固定する。

