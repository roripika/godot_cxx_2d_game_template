# phase4_entry_doc Report

## Summary

`project_handoff_memo.md` と既存 Phase 3-C 文書を基に、Phase 4 の入口条件・対象範囲・非対象範囲・Human Gate 維持条件を `docs/phase4_entry.md` に固定した。  
あわせて `docs/README.md` に参照リンクを追加し、入口文書の発見性を確保した。

## Files Changed

- `docs/phase4_entry.md`（新規）
- `docs/README.md`（`phase4_entry.md` への参照を 1 行追加）
- `reports/phase4_entry_doc_report.md`（本レポート）

## Key Decisions

- Phase 4 は「完全自動化」ではなく「制約付き半自動化の継続」と定義した
- HG-1〜HG-4 は維持し、Gate の短絡（自動修復ループ・自動 commit など）を禁止と明記した
- `scenarios/generated/` 隔離運用は継続とし、expected output 管理方針を維持した
- 次候補は handoff memo の A〜D を踏襲し、Entry 固定→共通化設計→入力改善→統合テスト強化の順で整理した

## Risks / Open Issues

- Phase 4 の「共通化」をどの粒度で実装するか（dispatcher 先行か、spec 正規化先行か）は未決定
- `tasks/` や task YAML 専用ディレクトリは現リポジトリに存在せず、今後必要なら配置方針の追加定義が必要
- 統合テスト強化時に expected output 差分基準をどこまで厳格化するかは別途合意が必要

## Suggested Next Step

1. `docs/phase4_entry.md` を基準に、Phase 4 の最初の実作業を「共通化設計メモ作成」か「統合テスト強化」のどちらにするか決定する  
2. 選択した作業に対して、Out of Scope を維持した task contract を 1 件起票する

