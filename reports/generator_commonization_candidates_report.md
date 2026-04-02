# generator_commonization_candidates Report

## Summary
- `docs/generator_commonization_candidates.md` を新規作成し、4 generator の共通化候補を優先度付きで整理した。
- 共通化候補と非候補を分離し、Phase 4 で安全に進める段階導入順（Step 1〜4）を定義した。
- `docs/phase4_backlog.md` の P4-01 を Done に更新した。
- `docs/README.md` に導線を追加した。

## Files Changed
- `docs/generator_commonization_candidates.md`: 新規作成。
- `docs/phase4_backlog.md`: P4-01 の Status を Done に更新。
- `docs/README.md`: 参照リンクを追加。
- `reports/generator_commonization_candidates_report.md`: 本レポートを新規作成。

## 変更理由
- 共通化を実装先行で進めると影響範囲が広がるため、先に候補・非候補・順序を固定する必要があった。
- 4 generator の共通パイプラインは存在するが、template 固有ロジックは差分が大きいため、境界を先に明文化する必要があった。

## 変更しなかった候補
- 候補A: 4 generator を即時に dispatcher へ一本化  
  理由: 現時点ではデバッグ負荷が上がるため、後段候補に留めた。
- 候補B: `validate_spec` を単一スキーマへ統合  
  理由: template 固有制約が多く、可読性と保守性が低下するリスクが高い。

## Tests Run
- `rg -n "C-01|C-02|C-03|C-04|C-05|C-06|非候補|段階導入" docs/generator_commonization_candidates.md`: pass
- `git diff -- docs/generator_commonization_candidates.md docs/phase4_backlog.md docs/README.md`: pass

## Result
- Generator 共通化候補整理メモが追加され、P4-01 の完了条件を満たした。

## Risks / Open Issues
- C-03（共通バリデータ部品）と P4-05（Spec 正規化）の境界は次タスクで Scope 明確化が必要。
- C-05（dispatcher）の導入タイミングを誤ると障害切り分けが難しくなる。

## 次に着手すべき1タスク
- `P4-04 generator_regression_expansion_plan`（共通化前に退行検知を強化する）。

