# Phase 3-A 完了メモ

**ステータス**: 完了 ✅  
**期間**: 2026-03-27 〜 2026-03-28  
**担当**: AI エージェント（GitHub Copilot）

---

## 目的

既存ゲームモジュール（mystery / roguelike / rhythm / billiards）のコードを解析し、
「AI が新しいゲームロジックを書く際に参照できるジャンル別テンプレート」を 4 本整備すること。

---

## 成果物一覧

| タスク | 成果物 | commit |
|:---:|:---|:---:|
| T1 | `docs/template_package_spec.md` — テンプレートパッケージ仕様定義 | `ae75d8f` |
| T2 | `examples/templates/turn_grid_basic/` — ターン制グリッドゲームテンプレート | `fd2c78d` |
| T3 | `examples/templates/branching_basic/` — 分岐判定テンプレート<br>`docs/mystery_test_completion.md` — mystery モジュール完了メモ | `d9d4002` |
| T4 | `examples/templates/time_clock_basic/` — 時刻・リズムゲームテンプレート | `8023e0a` |
| T5 | `docs/template_selection_guide.md` — 4 テンプレート横断選択ガイド | `bd757c8` |
| T6 | `examples/templates/event_driven_basic/` — イベント駆動待機テンプレート<br>`docs/billiards_test_completion.md` — billiards モジュール完了メモ | `d28c704` |

---

## 元計画との差分

元の `phase3_a_execution_plan.md` では T1〜T5（3 テンプレート）を計画していた。
billiards_test の解析時に `TaskResult::Waiting` + Fake-event タイムアウトという**既存 3 テンプレートにない**新カテゴリが確認されたため、
T6（event_driven_basic）を追加で formalize した。**当初計画を 1 テンプレート超過達成。**

---

## 4 テンプレートの最終整合確認結果

整合確認日: 2026-03-28  
確認基準: 各テンプレートに `README.md / contract.md / verification.md / scenario/*.yaml` の 4 点セットが揃っていること

```
branching_basic:    OK  |  scenario/branching_smoke.yaml
turn_grid_basic:    OK  |  scenario/turn_grid_smoke.yaml
time_clock_basic:   OK  |  scenario/time_clock_smoke.yaml
event_driven_basic: OK  |  scenario/event_driven_smoke.yaml
```

全 4 テンプレート：4 点セット揃い ✅

---

## 4 テンプレートの識別特徴

共通原則：
- `src/core` 改変禁止
- YAML シナリオ + Task クラスのみでロジック拡張
- pos-0 にダミータスクを置いて初回 skip を回避

| テンプレート | 元ゲーム | 入力モック | `TaskResult::Waiting` | `KernelClock` |
|:---|:---|:---|:---:|:---:|
| `branching_basic` | mystery_test | 不要（WorldState 直接書き込み） | なし | なし |
| `turn_grid_basic` | roguelike_test | FakeCommand | なし | なし |
| `time_clock_basic` | rhythm_test | FakeTap | なし | advance + now |
| `event_driven_basic` | billiards_test | タイムアウト Fake-event | **あり** | now のみ |

---

## Phase 3-B への引き継ぎ事項

Phase 3-B の内容は `docs/phase3_planning.md` の「2.2 AI Assist Tooling」セクションを参照。
優先順位:

1. **Task Catalog Auto-gen**（最優先）  
   `src/games/*/tasks/` を走査し、登録済み全 Task の名前・ペイロードキー・説明を Markdown に自動出力するスクリプトを作成する。

2. **`validate_scenario.py` 拡張**  
   現状のバリデーション（エラー検出）に加え、「修正案の提示」と「NL によるエラー解説（Schema-driven Guidance）」を追加する。

3. **Generator Design**（将来）  
   Few-shot Prompts 設計・Iterative Refinement Flow（スコープ外のため優先度低）。

---

## 既知の未解決事項

| 項目 | 状況 | 影響 |
|:---|:---|:---|
| pos-0 skip の runtime 実測 | billiards_test での確認未実施 | 既存 workaround（ダミー配置）で回避中 |
| event_driven: `event:last_name` の runtime 実測 | コード解析のみ、実ゲームでの動作確認未実施 | テスト環境整備後に確認推奨 |
| billiards_test の物理コリジョン精度 | BilliardsManager 側の問題（core 外） | テンプレートのスコープ外 |

---

## 参照ドキュメント

- [`template_package_spec.md`](template_package_spec.md) — テンプレートパッケージ仕様
- [`template_selection_guide.md`](template_selection_guide.md) — 4 テンプレート比較・選択フロー
- [`mystery_test_completion.md`](mystery_test_completion.md) — mystery モジュール完了メモ
- [`billiards_test_completion.md`](billiards_test_completion.md) — billiards モジュール完了メモ
- [`phase3_planning.md`](phase3_planning.md) — Phase 3 全体計画（Phase 3-B の詳細）
