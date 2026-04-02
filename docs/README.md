# Karakuri Project — docs ゲートウェイ

AI エージェント・人間担当者とも、**まずこのファイルを読んでから** 必要な docs を選んでコンテキストに取り込んでください。

> **AI への最重要命令**:
> `src/core/` は凍結済みです。拡張は `src/games/` 配下の Task クラスと YAML シナリオのみで行ってください。

---

## 読み方ガイド（3層）

### 層A — 常時参照（3本のみ）

どんな作業でも常にコンテキストに入れる最小セット。

| ファイル | 内容 |
| :--- | :--- |
| [`../ARCHITECTURE.md`](../ARCHITECTURE.md) | Kernel v2.0 の基本原則・制約 |
| [`kernel_test_matrix.md`](kernel_test_matrix.md) | 全モジュールの現状とカバレッジ地図 |
| **この README.md** | ゲートウェイ（本ファイル） |

### 層B — 必要時のみ参照

特定のモジュールや移行作業をする場合のみ読む。

**フェーズ資料**（フェーズ移行時のみ）

- [`phase2_summary.md`](phase2_summary.md) — 基盤フェーズの到達点
- [`infrastructure_backlog.md`](infrastructure_backlog.md) — 既知制約と回避策（pos-0 skip など）
- [`phase3_planning.md`](phase3_planning.md) — Phase 3 全体ロードマップ
- [`phase3_a_execution_plan.md`](phase3_a_execution_plan.md) — Phase 3-A 実行計画
- [`template_package_spec.md`](template_package_spec.md) — Template Package 最小構成定義（Phase 3-A T1）
- [`../examples/templates/turn_grid_basic/`](../examples/templates/turn_grid_basic/) — Turn/Grid Basic テンプレート 4 点セット（Phase 3-A T2）
- [`../examples/templates/branching_basic/`](../examples/templates/branching_basic/) — Branching Basic テンプレート 4 点セット（Phase 3-A T3）
- [`../examples/templates/time_clock_basic/`](../examples/templates/time_clock_basic/) — Time/Clock Basic テンプレート 4 点セット（Phase 3-A T4）
- [`template_selection_guide.md`](template_selection_guide.md) — 4 テンプレート横断選択ガイド・比較表・移行パス（Phase 3-A T5、T6 更新済み）
- [`../examples/templates/event_driven_basic/`](../examples/templates/event_driven_basic/) — Event-Driven Basic テンプレート 4 点セット（Phase 3-A T6）
- [`phase3_a_completion.md`](phase3_a_completion.md) — Phase 3-A 完了メモ（T1〜T6、2026-03-28）
- [`task_catalog.md`](task_catalog.md) — Task カタログ（自動生成 / 26 tasks / Phase 3-B T7）`tools/gen_task_catalog.py` で再生成。
- [`validate_scenario_guidance.md`](validate_scenario_guidance.md) — validate_scenario.py Guidance 仕様（エラー種別・SUGGEST 対応表 / Phase 3-B T9）
- [`few_shot_prompts.md`](few_shot_prompts.md) — ⚠️ **YAML 作成時のみ参照**（常時参照不要）4 テンプレート × 入力例・選択理由・最小 YAML・よくある間違い（Phase 3-B T10）
- [`phase3_b_completion.md`](phase3_b_completion.md) — Phase 3-B 完了メモ（AI Assist Tooling T7〜T10 / 到達点・限界・次段への入口、2026-03-31）
- [`phase3_c_generator_entry.md`](phase3_c_generator_entry.md) — Phase 3-C Generator 入口設計メモ（スコープ定義・Human Gate・Branching First 戦略、2026-04-01）
- [`generator_spec_schema.md`](generator_spec_schema.md) — Branching Basic 用 Structured Spec 入力スキーマ（V-01〜V-11 バリデーション規則・入力→YAML マッピング、2026-04-01）
- [`t11_gen_branching_completion.md`](t11_gen_branching_completion.md) — T11 完了メモ（gen_scenario_branching.py / 入力スコープ・自動補完・HG依存・smoke結果・既知制約、2026-04-01）
- [`generated_dir_policy.md`](generated_dir_policy.md) — `scenarios/generated/` 運用方針（review.md 位置づけ・git ルール・ライフサイクル・クリーンアップ、2026-04-01）
- [`generator_spec_schema.md`](generator_spec_schema.md) — Branching Basic + **Turn/Grid Basic 入口設計**（Spec スキーマ・V-TG-01〜V-TG-09・マッピング規則・R-1/R-4 確定済み、2026-04-01 更新）
- [`t12_gen_turn_grid_completion.md`](t12_gen_turn_grid_completion.md) — T12 完了メモ（gen_scenario_turn_grid.py / R-1 smoke 結果・出力仕様・既知制限、2026-04-01）
- [`generator_spec_schema.md`](generator_spec_schema.md) — **Time/Clock Basic 入口設計追加**（T13 節: V-TC-01〜V-TC-12・R-5/R-6/R-7 未解決リスク・全シーン sacrifice マッピング、2026-04-01 更新）
- [`t13_gen_time_clock_completion.md`](t13_gen_time_clock_completion.md) — T13 完了メモ（gen_scenario_time_clock.py / R-5/R-6/R-7 smoke 結果・出力仕様・既知制限、2026-04-01）
- [`generator_spec_schema.md`](generator_spec_schema.md) — **Event-Driven Basic 入口設計追加**（T14 節: V-ED-01〜V-ED-12・R-8/R-9/R-10 未解決リスク・continue/terminal sacrifice マッピング、2026-04-01 更新）
- [`t14_gen_event_driven_completion.md`](t14_gen_event_driven_completion.md) — T14 完了メモ（gen_scenario_event_driven.py / R-8/R-9/R-10 smoke 結果・出力仕様・既知制限、2026-04-01）
- [`phase3_c_completion.md`](phase3_c_completion.md) — **Phase 3-C 完了レポート**（4 Generator 到達点・共通アーキテクチャ・pos-0 sacrifice 統一規則・HG 依存・限界・次段候補 A〜D、2026-04-01）
- [`phase4_entry.md`](phase4_entry.md) — **Phase 4 入口定義メモ**（開始条件・in-scope/out-of-scope・Human Gate 維持・generated 隔離継続・次候補優先順、2026-04-02）
- [`phase4_backlog.md`](phase4_backlog.md) — **Phase 4 実行バックログ**（小タスク連続投入の優先順・Deliverables・Done/保留条件、2026-04-02）
- [`human_gate_decision_matrix.md`](human_gate_decision_matrix.md) — **Human Gate 判定表**（HG-1〜HG-4 の PASS/REWORK/STOP 条件・証跡・テンプレート別 HG-2 重点、2026-04-02）
- [`generator_commonization_candidates.md`](generator_commonization_candidates.md) — **Generator 共通化候補整理**（共通化候補/非候補・優先度・段階導入順、2026-04-02）

**モジュール設計・完了メモ**（触るモジュールのファイルのみ読む）

| モジュール | 設計書 | 完了メモ |
| :--- | :--- | :--- |
| mystery_test | [`mystery_design.md`](mystery_design.md) | [`mystery_test_completion.md`](mystery_test_completion.md) |
| roguelike_test | [`roguelike_test_design.md`](roguelike_test_design.md) | [`roguelike_test_completion.md`](roguelike_test_completion.md) |
| rhythm_test | [`rhythm_test_design.md`](rhythm_test_design.md) | [`rhythm_test_completion.md`](rhythm_test_completion.md) |
| billiards_test | — | [`billiards_test_completion.md`](billiards_test_completion.md) |

**将来設計**（該当タスク時のみ）

- [`design/real_input_bridge.md`](design/real_input_bridge.md)
- [`design/stress_test_strategy.md`](design/stress_test_strategy.md)

### 層C — 原則無視（人間向け記録）

AI は通常コンテキストに取り込まない。技術的負債の調査時のみ参照。

- **`docs/reports/`** — 監査レポート・差分監査・個別負債記録（ノイズになるため常時参照不要）
- [`walkthrough.md`](walkthrough.md) — 開発過程のプログレスログ

---

## ディレクトリ役割早見表

| パス | 役割 | AI への推奨 |
| :--- | :--- | :--- |
| `docs/` | 設計・仕様・完了メモ | 層A/B を選択的に読む |
| `docs/design/` | 将来設計（未実装） | 特定タスク時のみ |
| `docs/reports/` | 監査・検証履歴 | **原則無視** |
| `docs/images/screenshots/` | 実行結果スクリーンショット | 無視 |
