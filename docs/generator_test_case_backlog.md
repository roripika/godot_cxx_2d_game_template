# Generator Test Case Backlog

**ステータス**: v1.0（テストケース台帳）  
**作成日**: 2026-04-02  
**対象**: `tests/test_generators.py`, `tests/fixtures/`

---

## 1. 目的

Generator 回帰テストの実装候補を、優先順・期待結果・除外条件つきで固定する。  
本ドキュメントは P4-04 の拡張計画を、実装可能なケース台帳に落としたものである。

---

## 2. 実行優先順（リリース順）

1. Batch A（P0）: 最低限の異常系検知を追加
2. Batch B（P1）: 境界値と生成物品質検知を追加
3. Batch C（P2）: テストランナーの例外系を補完

---

## 3. テストケース台帳

| Case ID | Batch | 対象 | 期待結果 | 除外条件（このケースを実行しない条件） |
|:---|:---:|:---|:---|:---|
| GTC-01 | A | 正常系 4 generator（既存 fixture） | 全 generator が generate/validate/compare で PASS | 既存 fixture を更新中で expected が未確定のとき |
| GTC-02 | A | template 不一致 fixture（4種） | generator exit 1、YAML 未生成 | template 名追加作業中で許可値変更が未確定のとき |
| GTC-03 | A | 必須フィールド欠落 fixture（4種） | generator exit 1、rule id を含むエラー出力 | 必須項目定義そのものを改定中のとき |
| GTC-04 | B | 値範囲違反 fixture（turn_grid/time_clock/event_driven） | generator exit 1 | 範囲ルールの仕様改定PRが未確定のとき |
| GTC-05 | B | シーン名衝突 fixture（4種） | generator exit 1 | scene 命名規則の改定中で衝突判定が未確定のとき |
| GTC-06 | B | review 生成確認（4種） | `*_review.md` が生成される | review 生成自体を無効化する設計変更が承認されたとき |
| GTC-07 | B | `--generator <name>` 単体実行 | 指定した1件のみ実行し、終了コードが期待通り | テストランナーCLI改修中で引数仕様が未固定のとき |
| GTC-08 | C | 不正 generator 名 | exit 1 + エラーメッセージ | 引数バリデーション仕様を変更中のとき |
| GTC-09 | C | expected 差分検出（compare fail） | compare step が FAIL を返し差分を表示 | expected 更新作業中で意図的差分が混在する期間 |

---

## 4. ケース実装時の最小ルール

- `tests/test_generators.py` の既存正常系フローを壊さない
- 異常系 fixture は `tests/fixtures/` に追加し、命名で目的を明示する
- 1 PR で追加するのは 1 batch まで（差分拡大を防ぐ）
- エラー文言の完全一致比較は避け、終了コードと主要キーワードで判定する

---

## 5. 除外条件（共通）

以下の期間は、回帰テストの FAIL を品質劣化と即断しない。

1. template 入力仕様の改定中
2. expected output の更新作業中
3. テストランナー CLI の互換移行期間

上記期間でも、除外理由を report に明記すること。

---

## 6. 完了条件（P4-07）

1. ケースが優先順（Batch A/B/C）で整理されている
2. 各ケースに期待結果がある
3. 各ケースに除外条件がある

---

## 7. 次の実装タスクへの受け渡し

次タスクでは Batch A（GTC-01〜GTC-03）だけを実装対象にする。  
Batch B/C は Batch A 安定後に段階追加する。

---

## 8. 参照

- [`generator_regression_expansion_plan.md`](generator_regression_expansion_plan.md)
- [`phase4_backlog.md`](phase4_backlog.md)
- [`tests/test_generators.py`](../tests/test_generators.py)

