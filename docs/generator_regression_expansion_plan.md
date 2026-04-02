# Generator Regression Expansion Plan

**ステータス**: v1.0（計画）  
**作成日**: 2026-04-02  
**対象**: `tests/test_generators.py` と `tests/fixtures/`

---

## 1. 目的

4 generator の回帰検知を、現在の「最小 smoke（正常系中心）」から段階的に拡張する。  
本計画は実装ではなく、追加ケースと実行順の固定を目的とする。

---

## 2. 現状（基準）

現行 `tests/test_generators.py` は、各 generator に対して次の 3 step を実施している。

1. spec fixture で generator 実行（exit 0 / YAML 出力）
2. `validate_scenario.py` 実行（exit 0）
3. expected output との構造比較（一致）

対象 fixture:
- `tests/fixtures/spec_branching.yaml`
- `tests/fixtures/spec_turn_grid.yaml`
- `tests/fixtures/spec_time_clock.yaml`
- `tests/fixtures/spec_event_driven.yaml`

このため、現状は「正常系の退行」には強いが、「異常系の退行」検知は薄い。

---

## 3. 拡張方針

- 既存正常系テストを壊さず、異常系を段階追加する
- まず generator 単体で完結する失敗ケースを追加する
- その後、review 生成品質や CLI 入力エラーを追加する
- すべて `src/core` 非変更で完結させる

---

## 4. 追加回帰ケース（候補一覧）

| ID | 種別 | 対象 | 期待結果 | 優先度 |
|:---|:---|:---|:---|:---:|
| RGE-01 | 正常系再確認 | 4 generator | 現行 3 step が全件 PASS | P0 |
| RGE-02 | template 不一致 | 4 generator | generator exit 1（spec error） | P0 |
| RGE-03 | 必須フィールド欠落 | 4 generator | generator exit 1（spec error） | P0 |
| RGE-04 | 値範囲違反 | turn_grid / time_clock / event_driven | generator exit 1 | P1 |
| RGE-05 | シーン名衝突 | branching / turn_grid / time_clock / event_driven | generator exit 1 | P1 |
| RGE-06 | review 生成確認 | 4 generator | `*_review.md` が生成される | P1 |
| RGE-07 | `--generator <name>` 単体実行 | テストランナー | 指定1件のみ実行して終了 | P1 |
| RGE-08 | 不正 generator 名 | テストランナー | exit 1 + エラーメッセージ | P2 |
| RGE-09 | expected 差分検出 | テストランナー | compare step が FAIL を返す | P2 |

---

## 5. 実行順（導入ステップ）

1. Step 1（最優先）: RGE-01〜RGE-03
2. Step 2: RGE-04〜RGE-06
3. Step 3: RGE-07〜RGE-09

理由:
- Step 1 で最小の失敗検知を先に確立する
- Step 2 で生成物品質（review）と境界値を補強する
- Step 3 でテストランナー自体の回帰を補完する

---

## 6. 実装スコープ（次タスク向け）

許可:
- `tests/test_generators.py` の拡張
- `tests/fixtures/` の追加（異常系 fixture）
- 必要最小限のテスト補助関数追加

非許可:
- `src/core` の変更
- 4 generator 本体仕様の変更
- validator ロジック変更

---

## 7. 完了条件（P4-04）

1. 追加回帰ケースが ID 付きで定義されている
2. 実行順（Step 1〜3）が明記されている
3. 次タスクで触るファイル境界が明記されている

---

## 8. リスクと抑制

| リスク | 抑制策 |
|:---|:---|
| 異常系 fixture の作り込みで差分が広がる | Step 1 で最小ケースのみ先行投入 |
| generator 実装差分で共通テストが複雑化 | case ID ごとに対象 generator を明示 |
| expected 更新作業が混入する | P4-04 では「計画のみ」に限定し、実装タスクで分離 |

---

## 9. 参照

- [`phase4_backlog.md`](phase4_backlog.md)
- [`phase3_c_completion.md`](phase3_c_completion.md)
- [`generator_commonization_candidates.md`](generator_commonization_candidates.md)
- [`tests/test_generators.py`](../tests/test_generators.py)

