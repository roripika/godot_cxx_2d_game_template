# Summary
- Phase 4 次段として Batch B 実装（GTC-04〜GTC-07）を 1 タスクで実行できるよう、契約文書を新規作成した。
- `phase4_backlog.md` に P4-09 を `Ready` で追加し、直近実行順を「Batch B → spec 正規化最小実装 → 共通化 PoC」に更新した。
- これにより、docs レーン完了後の次段実装投入順がバックログ上で固定された。

## Files Changed
- `docs/p4_09_generator_regression_batch_b_contract.md`: Batch B 実装用 TASK EXECUTION CONTRACT を追加
- `docs/phase4_backlog.md`: P4-09 追加と直近実行順の更新

## 変更理由
- 中間レビューで推奨された優先順を、実行可能な契約単位へ落とし込むため。
- Batch A 実装成功後の次段（tests/fixtures 中心）を迷わず着手できる状態にするため。

## 変更しなかった候補
- 候補A: Batch B を事前起票せず、都度チャットで口頭指示して実装する。
  - 不採用理由: 再現性と停止条件の明確性が下がるため。
- 候補B: Batch B ではなく spec 正規化実装を先に起票する。
  - 不採用理由: 中間レビューの推奨順（Batch B 先行）と一致しないため。

## Tests Run
- `not run`（docs/backlog 更新のみのため）

## Result
- P4-09 の実行契約が作成され、バックログ上で次着手タスクが機械的に判定可能な状態になった。

## Risks / Open Issues
- Batch B 実装時、`tests/test_generators.py` に補助関数追加が増えすぎると Stop Conditions（実行モデル再設計）に接触する可能性がある。
- GTC-07（`--generator` 単体実行判定）は実装方法を保守的に設計する必要がある。

## 次に着手すべき1タスク
- `generator_regression_batch_b_implementation`（P4-09）を契約どおり実装する。
