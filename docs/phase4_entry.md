# Phase 4 Entry Memo

**ステータス**: Entry 定義確定  
**作成日**: 2026-04-02  
**前提資料**: `project_handoff_memo.md`, `docs/phase3_c_completion.md`, `docs/phase3_c_generator_entry.md`, `docs/generated_dir_policy.md`

---

## 1. 目的

Phase 4 で何を進めるかを開始時点で固定し、以下を明文化する。

- 開始条件（何が成立していれば着手可能か）
- 対象範囲 / 非対象範囲
- Human Gate（HG-1〜HG-4）維持方針
- 次フェーズ候補の優先順位

この文書は、Phase 4 での設計判断を既存方針から逸脱させないための入口定義である。

---

## 2. 前提（Phase 3-C 完了時点）

`project_handoff_memo.md` に基づき、以下は成立済みとみなす。

- Kernel v2.0
- validator / regression 最低ライン
- 4 fitness test modules
- 4 template formalization
- assist tooling
- 4 minimal generators
- 4 generator 統合 smoke（`tests/test_generators.py`）

参照:
- [`phase3_c_completion.md`](phase3_c_completion.md)
- [`phase3_c_generator_entry.md`](phase3_c_generator_entry.md)

---

## 3. Phase 4 開始条件

Phase 4 は、以下 4 点が満たされているため着手可能と定義する。

1. Template 層が成立している
2. Assist Tooling 層が成立している
3. Minimal Generator 層が成立している
4. Integration Smoke が成立している

同時に、以下も明示的に維持する。

- まだ完全自動 Generator / 無人オーケストレーション段階ではない
- Human Gate を前提とした制約付き半自動化を継続する

---

## 4. Phase 4 対象範囲（In Scope）

Phase 4 では「制約付き Generator」の境界を保ったまま、次の設計・改善を対象にする。

1. 4 Generator の共通化に向けた設計整理（実装前提の仕様化）
2. Structured Spec 入力の品質改善（書式統一・入力ミス低減）
3. Generator 回帰の強化（既存 smoke / expected output を使った退行検出）
4. 出力ライフサイクルの運用明確化（`scenarios/generated/` と review.md の扱い）

補足:
- いずれも `src/core` を触らずに進める
- まず設計と運用境界を固定し、実装は最小差分で段階投入する

---

## 5. Phase 4 非対象範囲（Out of Scope）

Phase 4 で扱わない事項を以下に固定する。

- `src/core` の変更
- generator 実装の大規模改修（全面書き換え・多機能化の一括導入）
- validator / regression のロジック変更
- runtime failure 自動修復ループ
- validator エラー自己修正ループ
- commit / push の自動実行
- 無人オーケストレーション導入
- 自由文入力からの直接生成（構造化 spec を介さない生成）

---

## 6. Human Gate 維持方針

Phase 4 でも HG-1〜HG-4 を維持し、Gate の短絡を禁止する。

| Gate | タイミング | 維持ルール |
|:---|:---|:---|
| HG-1 | template 選択後 | テンプレート選択妥当性を人間が承認 |
| HG-2 | YAML 生成後 | 生成 YAML / review.md を人間が確認 |
| HG-3 | validator 実行後 | exit 1 修正は人間判断で実施 |
| HG-4 | runtime 実行前 | 実行前の最終確認を人間が実施 |

補足:
- Generator は HG を案内できるが、通過判定は人間が持つ
- HG をバイパスする機能（自動 commit・自動修復ループ等）は導入しない

参照:
- [`phase3_c_generator_entry.md`](phase3_c_generator_entry.md)
- [`phase3_c_completion.md`](phase3_c_completion.md)

---

## 7. generated 隔離方針（継続）

`scenarios/generated/` 運用は Phase 4 でも継続する。

- 生成物は `scenarios/generated/` に隔離
- タイムスタンプ付き生成 YAML は原則 git 管理外
- `*_review.md` は HG-2 用一時メモ
- expected output サンプルのみ基準として管理
- HG-4 通過後に人間が明示的に採用判断する

参照:
- [`generated_dir_policy.md`](generated_dir_policy.md)

---

## 8. 次フェーズ候補（Phase 4 内の優先順）

優先順は handoff memo の候補 A〜D を踏襲し、以下を推奨する。

1. Entry 定義固定（本ドキュメント）
2. 共通化設計（dispatcher / 共通 spec / validator 呼び出し統一）
3. 入力改善（Structured Spec の記述性・検証補助）
4. 統合テスト強化（generator 回帰ケース拡充）

---

## 9. Exit 条件（Phase 4 Entry 完了判定）

以下を満たしたら「Phase 4 入口定義完了」とする。

1. 本文書で in-scope / out-of-scope が固定されている
2. HG-1〜HG-4 維持方針が明示されている
3. generated 隔離運用の継続条件が明示されている
4. 次に進む候補の優先順が定義されている

