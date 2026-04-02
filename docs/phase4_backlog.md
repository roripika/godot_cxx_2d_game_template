# Phase 4 Backlog

**ステータス**: active  
**作成日**: 2026-04-02  
**前提**: `docs/phase4_entry.md`, `AGENTS.md`, `docs/agent_contract_template.md`

---

## 1. 目的

Phase 4 を「小タスク連続投入」で進めるための実行バックログを定義する。  
各タスクは `TASK EXECUTION CONTRACT` で個別実行し、1タスク1目的を守る。

---

## 2. 運用ルール（固定）

- 全タスクで `src/core` は変更禁止
- Human Gate（HG-1〜HG-4）を弱めない
- generated 生成物は `scenarios/generated/` に隔離
- validator / regression の既存動作を壊さない
- report は `reports/<task_name>_report.md` を必ず作成

---

## 3. 優先度定義

- `P0`: 直近着手。Phase 4 の土台を固定する
- `P1`: P0 後に順次投入
- `P2`: P1 の結果次第で投入

---

## 4. Backlog（小タスク一覧）

| ID | Priority | Task Name | Status | Deliverables | Completion Criteria |
|:---|:---:|:---|:---:|:---|:---|
| P4-01 | P0 | `generator_commonization_candidates` | Done | `docs/generator_commonization_candidates.md` | 共通化候補・非候補・段階導入順が明記されている |
| P4-02 | P0 | `human_gate_decision_matrix` | Done | `docs/human_gate_decision_matrix.md` | HG-1〜HG-4 の判定条件・入力・停止条件が表形式で定義されている |
| P4-03 | P0 | `docs_readme_phase4_navigation` | Done | `docs/README.md`（導線整理） | Phase 4 関連 docs への最短導線が追加されている |
| P4-04 | P1 | `generator_regression_expansion_plan` | Ready | `docs/generator_regression_expansion_plan.md` | 追加すべき回帰ケースと実行順が定義されている |
| P4-05 | P1 | `spec_input_normalization_note` | Ready | `docs/spec_input_normalization_note.md` | Structured Spec の揺れと統一案が整理されている |
| P4-06 | P1 | `generated_lifecycle_check_sync` | Ready | `docs/generated_dir_policy.md`（必要最小更新） | HG-2/HG-3/HG-4 ライフサイクル記述が現運用と一致している |
| P4-07 | P2 | `generator_test_case_backlog` | Ready | `docs/generator_test_case_backlog.md` | テストケースの優先順・期待結果・除外条件が定義されている |
| P4-08 | P2 | `phase4_midpoint_review` | Blocked | `docs/phase4_midpoint_review.md` | P0/P1 の実績を反映した中間レビューが作成されている |

---

## 5. 直近実行順（推奨）

1. `P4-04 generator_regression_expansion_plan`
2. `P4-05 spec_input_normalization_note`
3. `P4-06 generated_lifecycle_check_sync`
4. `P4-07 generator_test_case_backlog`

---

## 6. タスク起票ルール

各タスク起票時は、`docs/agent_contract_template.md` をコピーし、以下を最低限埋める。

- `Task Name`
- `Objective`
- `Scope / Out of Scope`
- `Hard Constraints`
- `Expected Deliverables`
- `Completion Criteria`
- `Stop Conditions`

---

## 7. Done 判定（Backlog項目共通）

以下を満たしたら、当該タスクを Done にする。

1. 指定成果物が作成または更新されている
2. 変更差分が Scope 内に収まっている
3. `reports/<task_name>_report.md` が作成されている
4. report に「変更理由」「変更しなかった候補」「Risks」「次の1タスク」が記載されている

---

## 8. 保留条件（Backlog更新停止）

以下に該当した場合、次タスク投入前に保留して判断する。

- `src/core` 変更が必要になる
- HG を維持したままでは成立しない案しか残らない
- 複数の設計分岐があり優先案を決められない
- validator 失敗原因が既存不具合か新規変更か切り分け不能
