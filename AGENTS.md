# AGENTS.md

このファイルは、`godot_cxx_2d_game_template` を Codex で継続運用するためのリポジトリ共通ルールである。

## 1. 基本方針
- 運用は「引き継ぎベース」ではなく「契約ベース」で行う。
- 各タスクは必ず `TASK EXECUTION CONTRACT` を使って投入する。
- タスクは小さく刻む（docs / task / test 単位）。

## 2. 必須テンプレート
- 契約テンプレート: `docs/agent_contract_template.md`
- レポートテンプレート: `reports/_report_template.md`

## 3. 実行フロー（Codex向け）
1. 契約の Scope / Out of Scope / Hard Constraints を確認する。
2. 関連資料を読み、最小差分で変更する。
3. Stop Conditions に該当したら実装を止めて報告する。
4. `reports/<task_name>_report.md` を作成する。

## 4. レポート必須項目
各レポートは最低限、以下を含める。

- Summary
- Files Changed
- 変更理由
- 変更しなかった候補
- Risks / Open Issues
- 次に着手すべき1タスク

## 5. 小タスク分割ルール
- 1タスク 1目的を原則とする。
- 一度に広範囲の設計変更・実装変更をしない。
- 例:
  - `phase4_backlog.md` 作成
  - generator 共通化候補の整理
  - Human Gate 判定表の明文化
  - `docs/README.md` の導線整理

## 6. 固定制約（現行運用）
- `src/core` は原則変更禁止（契約で明示許可がある場合のみ例外）。
- Human Gate（HG-1〜HG-4）を弱めない。
- generated 生成物は `scenarios/generated/` に隔離する。
- validator / regression の既存動作を壊さない。
- commit / push の自動実行はしない。

## 7. 変更ポリシー
- まず読む、次に最小変更で実装する。
- 推測で広げない。曖昧な場合は保守的に進める。
- 不要な命名変更・整形変更・広域リファクタリングをしない。

