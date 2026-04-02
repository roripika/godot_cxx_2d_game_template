# spec_input_normalization_note Report

## Summary
- `docs/spec_input_normalization_note.md` を新規作成し、4 generator の入力キー揺れを整理した。
- Canonical v2（共通コア + `spec.*`）の正規化方針を定義した。
- v1/v2 互換運用（read 両対応、write 段階移行）を明記した。
- `docs/phase4_backlog.md` の P4-05 を Done に更新した。

## Files Changed
- `docs/spec_input_normalization_note.md`: 新規作成。
- `docs/phase4_backlog.md`: P4-05 を Done 化、直近実行順を更新。
- `docs/README.md`: 参照リンクを追加。
- `reports/spec_input_normalization_note_report.md`: 本レポートを新規作成。

## 変更理由
- 共通化・回帰強化を進める前提として、入力キーの不統一を先に可視化する必要があった。
- 既存 v1 spec を壊さずに移行するため、互換前提の段階導入方針を固定する必要があった。

## 変更しなかった候補
- 候補A: 直ちに generator 実装を Canonical v2 へ変更する  
  理由: P4-05 は方針固定タスクであり、実装変更は次タスクに分離するため。
- 候補B: v1 spec を即廃止する  
  理由: 既存 fixture/運用への影響が大きく、段階移行方針に反するため。

## Tests Run
- `rg -n "Canonical v2|v1/v2|spec\\.|scenes\\.|terminal_result" docs/spec_input_normalization_note.md`: pass
- `nl -ba docs/phase4_backlog.md | sed -n '34,60p'`: pass

## Result
- P4-05 の完了条件（入力揺れ整理・統一案定義）を満たした。

## Risks / Open Issues
- turn_grid の `terminal_*` top-level 形式を `scenes.*` に寄せる実装時に、既存 fixture 互換テストが必要。
- `meta.description` と既存 `description` の優先順位ルールは実装タスクで確定が必要。

## 次に着手すべき1タスク
- `P4-06 generated_lifecycle_check_sync` で generated 運用記述の整合を最新化する。

