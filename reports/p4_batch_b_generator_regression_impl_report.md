# Summary
- Batch B（GTC-04〜GTC-07）を `tests/test_generators.py` に段階追加した。
- 既存の Batch A 構造（`run_test()` と `run_negative_test()` の分離）を維持し、全面再設計は行っていない。
- GTC-04（値範囲違反）、GTC-05（シーン名衝突）、GTC-06（review 生成確認）、GTC-07（`--generator <name>` 単体実行）を追加し、既存 GTC-01〜GTC-03 と同時に検証できる状態にした。

# Files Changed
- `tests/test_generators.py`: Batch B 用 fixture マップ、review 生成確認、CLI 単体実行確認、集計出力を最小追加
- `tests/fixtures/spec_turn_grid_value_out_of_range.yaml`: GTC-04 用（turn_grid 範囲違反）
- `tests/fixtures/spec_time_clock_value_out_of_range.yaml`: GTC-04 用（time_clock 範囲違反）
- `tests/fixtures/spec_event_driven_value_out_of_range.yaml`: GTC-04 用（event_driven 範囲違反）
- `tests/fixtures/spec_branching_scene_collision.yaml`: GTC-05 用（branching シーン名衝突）
- `tests/fixtures/spec_turn_grid_scene_collision.yaml`: GTC-05 用（turn_grid シーン名衝突）
- `tests/fixtures/spec_time_clock_scene_collision.yaml`: GTC-05 用（time_clock シーン名衝突）
- `tests/fixtures/spec_event_driven_scene_collision.yaml`: GTC-05 用（event_driven シーン名衝突）
- `docs/phase4_backlog.md`: P4-09 を Done 化、直近実行順を次段タスク向けに更新

# 変更理由
- `generator_test_case_backlog.md` の優先順に従い、Batch A 安定後の次段として Batch B を 1 タスクで実装するため。
- 既存正常系/Batch A の回帰検知を維持しつつ、異常系境界と生成物品質、テストランナー CLI の最低限検知を追加するため。

# 変更しなかった候補
- 候補A: `tests/test_generators.py` をケース駆動の汎用フレームワークに再設計する。
  - 不採用理由: 本タスクの制約（Batch A 構造を壊さず最小差分で段階追加）に反するため。
- 候補B: GTC-07 のために test runner CLI 仕様を変更する。
  - 不採用理由: Out of Scope かつ既存 `--generator` 仕様の検証だけで要件を満たせるため。

# Tests Run
- `.temp_venv/bin/python3 tests/test_generators.py`: pass（27/27 checks passed）

# Result
- GTC-01（既存正常系）: PASS
- GTC-02/GTC-03（Batch A）: PASS
- GTC-04（値範囲違反）: PASS（対象 3 generator が non-zero + YAML 未生成）
- GTC-05（シーン名衝突）: PASS（対象 4 generator が non-zero + YAML 未生成）
- GTC-06（review 生成確認）: PASS（対象 4 generator で `*_review.md` 生成確認）
- GTC-07（`--generator <name>` 単体実行）: PASS（指定 1 generator のみ実行、exit 0）

# Risks / Open Issues
- GTC-07 はテストランナー自己呼び出しで検証しているため、将来 CLI オプションが増えた場合は `--skip-gtc07` 連携の見直しが必要。
- review 生成確認はファイル存在確認のみで、review 内容品質は未検証（次段で必要なら別ケース化）。

# 次に着手すべき1タスク
- `spec_input_normalization` の最小実装タスクを起票して、v1/v2 入力揺れ吸収を段階導入する。
