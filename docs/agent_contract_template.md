# TASK EXECUTION CONTRACT Template

このテンプレートをコピーし、`<...>` を埋めてから Codex に渡す。

---

## 0. Task Name
`<short_task_name>`

## 1. Objective
このタスクの目的を 1-3 行で記載する。

## 2. Background
現状の到達点、前提、依存関係を記載する。

## 3. Scope
このタスクで実施してよい範囲を列挙する。

## 4. Out of Scope
このタスクで実施してはいけない範囲を列挙する。

## 5. Hard Constraints
必ず守る制約を列挙する。

## 6. Inputs
参照すべき資料・ファイルを列挙する。

## 7. Expected Deliverables
最終成果物のファイルパスを明示する。

## 8. Work Steps
1. 調査
2. 境界確認
3. 最小差分で実装
4. 検証
5. レポート作成

## 9. Completion Criteria
機械的に判定できる完了条件を列挙する。

## 10. Stop Conditions
該当したら停止して報告する条件を列挙する。

## 11. Output Format
`reports/<task_name>_report.md` を作成し、以下を最低限含めること。

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
- 推測で広げない
- 不要な命名変更や整形変更をしない
- 説明可能な変更のみ行う
- 曖昧な場合は保守的に進める

## 13. Instruction to Codex
上記制約を厳守し、このタスクを完了してください。  
禁止範囲に触れる必要が生じた場合は実装を中断し、その理由を報告してください。

