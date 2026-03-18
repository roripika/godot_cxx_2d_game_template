# プロジェクト開発状況 (Development Status)

本ドキュメントは、プロジェクトの現在の進捗、実装済み機能、および内部タスクへのリンクをまとめたものです。

## 現在のフェーズ (Current Phase)
**Phase 2: Kernel Fitness Test（整理完了）** — 2026-03-19

> 詳細レポート: [docs/reports/phase2_diff_audit_report.md](docs/reports/phase2_diff_audit_report.md)

### Phase 2 完了サマリー
| コミット | 内容 |
|---|---|
| `a6c5364` | fix(phase2): 差分整理 — Core 変更を差し戻し、A 分類のみコミット |
| `121f686` | feat: billiards_test 最小骨格 (4 タスク) + validator ガードレール |
| `a9da471` | fix: validator 9 アクション同期 + diagnostic_test.yaml |

#### 確定済み（Category A）
- `mystery_test` 未追跡タスク群をコミット（parallel / save_load / wait_for_signal）
- `ScenarioValidator` C++ 実装 + `register_types.cpp` 登録
- `check_condition_task.cpp` UB バグ修正（return 文欠如）
- `billiards_test` タスクの `const char*` クラッシュ対策
- `tools/test_validator.py` 回帰テスト 7 ケース全 PASS

#### 保留（Category B — 別 PR 候補）
- `src/core/action_registry` の GDExtension 起動クラッシュ対策（Core 変更が必要なため保留）

---

**Phase 5.5 / Phase 6 ステータス（旧記録）**

## 実装済み機能 (Implemented Features)

### 1. Adventure Mode (アドベンチャーモード)
**ステータス: 完了 (C++移行済み)**
- 全ロジックを GDScript から **C++ (GDExtension)** に移行しました。
- **主要クラス**:
    - `AdventureGameStateBase`: フラグ・インベントリ・シーン遷移管理。
    - `OfficeSceneLogic`: オフィスシーンのロジック。
    - `HauntedSpotSceneLogic`: 幽霊スポットシーンのロジック。
    - `InteractionManager`: クリックイベントの検知とシグナル発行。
    - `DialogueUI`: 会話ウィンドウの表示と制御。

### 2. Asset Generation (アセット生成)
**ステータス: 進行中 (9/12体完了)**
- 生成AI (Gemini) を活用したドット絵素材の作成フローを確立しました。
- **パイプライン**:
    1. Geminiで画像生成 (PNG)
    2. `potrace` でベクター化 (SVG)
    3. `scripts/resize_svg.py` で指定サイズ (64x64) にリサイズしてPNG出力
- **課題**: API制限のため、残り3体（酉・戌・亥）の生成が保留中。

## 開発ドキュメント (Internal Documentation)
詳細なタスク管理、設計、実装計画は以下のブレイン・アーティファクトで管理されています。

- **タスク一覧 (Task List)**
    - [task.md](file:///Users/ooharayukio/.gemini/antigravity/brain/d49ea601-db01-4a62-b20b-e219080009a8/task.md)
    - 全体のプログレスと未消化タスク（絵の課題など）を管理。

- **実装計画書 (Implementation Plan)**
    - [implementation_plan.md](file:///Users/ooharayukio/.gemini/antigravity/brain/d49ea601-db01-4a62-b20b-e219080009a8/implementation_plan.md)
    - 各フェーズの詳細な実装ステップ。

- **設計書 (Design Doc)**
    - [design_doc_universal_template.md](file:///Users/ooharayukio/.gemini/antigravity/brain/d49ea601-db01-4a62-b20b-e219080009a8/design_doc_universal_template.md)
    - ユニバーサルゲームテンプレートのアーキテクチャ設計。

- **ウォークスルー (Walkthrough)**
    - [walkthrough.md](file:///Users/ooharayukio/.gemini/antigravity/brain/d49ea601-db01-4a62-b20b-e219080009a8/walkthrough.md)
    - 検証手順と結果の記録。
