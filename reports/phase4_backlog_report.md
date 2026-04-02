# phase4_backlog Report

## Summary
- Phase 4 を小タスク連続投入で進めるための実行バックログを `docs/phase4_backlog.md` として新規作成した。
- バックログに Priority（P0/P1/P2）、Status（Ready/Blocked）、Deliverables、Completion Criteria を固定した。
- `AGENTS.md` と `docs/agent_contract_template.md` の運用に合わせ、契約駆動の起票ルールと Done 判定を明文化した。
- `docs/README.md` に Phase 4 backlog への導線を追加した。

## Files Changed
- `docs/phase4_backlog.md`: Phase 4 実行バックログを新規追加。
- `docs/README.md`: `phase4_backlog.md` へのリンクを 1 行追加。
- `reports/phase4_backlog_report.md`: 本レポートを新規追加。

## 変更理由
- Phase 4 を大タスクで進めると Scope 逸脱リスクが高いため、1タスク1目的の分割運用を先に固定する必要があった。
- TASK EXECUTION CONTRACT 運用を毎回再設計せずに回せるよう、Ready 状態の候補タスクを事前に定義した。
- Human Gate 維持・`src/core` 非変更など既存制約を、各タスクで再利用できる形に落とし込むため。

## 変更しなかった候補
- 候補A: `tasks/` ディレクトリを新設して backlog を管理する  
  理由: 現在リポジトリに `tasks/` は存在せず、今回タスクは docs 作成が主目的のため見送った。
- 候補B: backlog と同時に `P4-01` 〜 `P4-03` を一括実装する  
  理由: 1タスク1目的の運用原則に反するため、今回は起票可能状態の整理に限定した。

## Tests Run
- `git status --short docs/phase4_backlog.md docs/README.md reports/phase4_backlog_report.md`: pass
- `nl -ba docs/phase4_backlog.md | sed -n '1,260p'`: pass

## Result
- `docs/phase4_backlog.md` が作成され、Phase 4 の小タスク実行計画が運用可能な形で定義された。
- `docs/README.md` から backlog へ到達できるようになった。
- 変更は docs/report のみに限定され、実装コードには未変更。

## Risks / Open Issues
- P4-08（midpoint review）は前提タスクの実績が必要なため、現時点では Blocked のまま。
- P4-01 と P4-05 の境界（共通化候補と Spec 正規化）に重なりがあるため、起票時に Scope を厳密化する必要がある。

## 次に着手すべき1タスク
- `P4-01 generator_commonization_candidates` を契約テンプレートで起票し、共通化対象/非対象を文書化する。

