# Summary

Batch A（GTC-01〜GTC-03）として、`tests/test_generators.py` に負ケース実行経路を最小追加し、
4 generator それぞれで以下を確認できるようにした。

- GTC-01: 既存正常系（generate / validate / compare）が継続 PASS
- GTC-02: template 不一致で non-zero exit + YAML 未生成
- GTC-03: 必須項目欠落で non-zero exit + YAML 未生成 + 失敗理由ヒント確認

変更理由:
- 既存 `run_test()` の正常系フローを維持しながら、負ケース専用の実行経路を分離して追加するため。

変更しなかった候補:
- `run_test()` を汎用テストランナーへ全面再設計する案（Batch A の最小差分方針に反するため不採用）。
- generator 本体側にテスト用フラグや出力制御を追加する案（Out of Scope のため不採用）。

# Files Changed

- `tests/test_generators.py`
- `tests/fixtures/spec_branching_invalid_template.yaml`
- `tests/fixtures/spec_turn_grid_invalid_template.yaml`
- `tests/fixtures/spec_time_clock_invalid_template.yaml`
- `tests/fixtures/spec_event_driven_invalid_template.yaml`
- `tests/fixtures/spec_branching_missing_required.yaml`
- `tests/fixtures/spec_turn_grid_missing_required.yaml`
- `tests/fixtures/spec_time_clock_missing_required.yaml`
- `tests/fixtures/spec_event_driven_missing_required.yaml`

# Key Decisions

- 負ケースは `run_negative_test()` / `step_expect_generate_failure()` に分離し、正常系の既存ステップ構造を維持。
- 失敗判定は文言完全一致を避け、終了コード・YAML未生成・主要キーワード（`SPEC ERROR` / `[V-` / `rule`）で判定。
- fixture は既存正常系をベースに、原因を1点に絞った最小改変（template不一致 or 必須欠落）のみ追加。

# Risks / Open Issues

- 失敗理由判定はキーワードベースのため、将来エラーメッセージ規約が大きく変わると GTC-03 が要更新。
- テスト実行は `.temp_venv/bin/python3` を使用した（システム `python3` では PyYAML 未導入環境あり）。

# Suggested Next Step

1. Batch B（GTC-04〜GTC-07）を 1 タスクで起票し、境界値違反と review 生成確認を段階追加する。
