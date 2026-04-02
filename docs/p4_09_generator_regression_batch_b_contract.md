# TASK EXECUTION CONTRACT

## 0. Task Name
`generator_regression_batch_b_implementation`

## 1. Objective
Batch B（GTC-04〜GTC-07）を 1 タスクで実装し、Generator 回帰テストの異常系・補助検証を拡張する。

## 2. Background
- Phase 4 docs/planning/governance レーンは完了済み（P4-01〜P4-08 Done）。
- Batch A（GTC-01〜GTC-03）は `ea0fed9` で実装済み、12/12 PASS を確認済み。
- 次段は `docs/generator_test_case_backlog.md` の優先順に従い、Batch B を実装する。

## 3. Scope
- `tests/test_generators.py` の最小差分拡張
- `tests/fixtures/` への Batch B 用 fixture 追加
- 必要最小限のテスト補助関数追加（負ケース・補助検証用）
- `reports/generator_regression_batch_b_implementation_report.md` の作成

## 4. Out of Scope
- `src/core` の変更
- generator 本体（`tools/gen_scenario_*.py`）の仕様変更
- validator / regression ロジック変更
- runtime failure 自動修復
- 無人オーケストレーション導入

## 5. Hard Constraints
- `src/core` は変更禁止
- Human Gate（HG-1〜HG-4）を弱めない
- 既存 GTC-01〜GTC-03 の成立を壊さない
- fixture 追加は `tests/fixtures/` に限定
- 推測で `tests/test_generators.py` 全体を再設計しない

## 6. Inputs
- `docs/generator_test_case_backlog.md`
- `docs/generator_regression_expansion_plan.md`
- `docs/phase4_midpoint_review.md`
- `tests/test_generators.py`
- `tests/fixtures/`
- `reports/_report_template.md`

## 7. Expected Deliverables
- `tests/test_generators.py`（Batch B 対応）
- `tests/fixtures/` 配下の Batch B fixture 群
- `reports/generator_regression_batch_b_implementation_report.md`

## 8. Work Steps
1. 既存 Batch A 実装と Batch B 要件（GTC-04〜GTC-07）を照合する
2. 値範囲違反 / シーン名衝突 fixture を最小差分で追加する
3. `tests/test_generators.py` に Batch B 判定ロジックを最小追加する
4. 回帰テストを実行して既存ケースの非退行を確認する
5. 結果と除外条件を report に記録する

## 9. Completion Criteria
- GTC-01〜GTC-03（既存）が全件 PASS
- GTC-04（値範囲違反）で対象 generator が non-zero exit を返し、YAML 未生成
- GTC-05（シーン名衝突）で対象 generator が non-zero exit を返し、YAML 未生成
- GTC-06 で正常系実行時に `*_review.md` 生成を検知できる
- GTC-07 で `--generator <name>` 単体実行が 1 件のみを対象にできる
- 変更差分が scope 内に収まる
- report に「変更理由」「変更しなかった候補」「Risks」「次の1タスク」がある

## 10. Stop Conditions
- `src/core` の変更が必要になった
- 負ケース拡張のために `tests/test_generators.py` の実行モデル再設計が必要になった
- generator エラー出力が不足し、主要失敗理由を機械的に確認できない
- 既存 GTC-01〜GTC-03 が恒常的に崩れ、原因切り分けが不能

## 11. Output Format
`reports/generator_regression_batch_b_implementation_report.md` に以下を記載する。

- Summary
- Files Changed
- 変更理由
- 変更しなかった候補
- Tests Run
- Result
- Risks / Open Issues
- 次に着手すべき1タスク

## 12. Execution Policy
- まず読む、次に最小変更で実装する
- 1 回の変更で 1 目的（Batch B）を守る
- 不要な命名変更・整形変更をしない
- 曖昧な仕様は保守的に解釈し、未確定部分は report に残す

## 13. Instruction to Codex
上記制約を厳守して Batch B（GTC-04〜GTC-07）を実装してください。  
完了後は report を作成し、対象差分のみコミットしてください。  
push は、禁止範囲に触れず、テスト結果が期待どおりで、Stop Conditions に該当しない場合に限って実施してください。  
禁止範囲に触れる必要が生じた場合は実装を中断し、理由を報告してください。
