# Mystery Demo 引き継ぎ資料（antigravity）

## 1. 目的
- Mystery デモを `docs/mystery_design.md` 準拠で完成度を上げる。
- 実装は「再利用前提」で行い、デモ固有ロジックは `samples/mystery/**`、共通ロジックは Basic Game Karakuri（`src/karakuri/**`）に分離する。
- 今後の作業者は antigravity に固定する。

## 2. 現在の前提（2026-02-17 時点）
- 実行入口: `project.godot` の `run/main_scene` は `res://samples/mystery/karakuri_mystery_shell.tscn`。
- シナリオ実行: `KarakuriScenarioRunner` が `samples/mystery/scenario/mystery.yaml` を読む。
- 設計書: `docs/mystery_design.md`。
- 進捗チェック一次情報: `TASK.md` の `Priority: Mystery Demo Execution (Owner: antigravity)` セクション。

## 3. 非機能ルール（必須）
- `src/karakuri/**` の C++ は Doxygen 形式コメント必須。
- Basic Game Karakuri にデモ固有ロジックを入れない。
- 文字列は原則翻訳キー経由で扱い、直書きを避ける。
- PR には最低限 `Scope / Out of Scope / Test` を記載する。

## 4. 役割分担
- Designer:
  - 主編集対象: `samples/mystery/ui/**`、各 `.tscn` の見た目、テーマ。
  - 責務: レイアウト、視認性、操作導線、UIアニメーション。
- Planner:
  - 主編集対象: `samples/mystery/scenario/**`、翻訳データ（CSV/translation）。
  - 責務: シナリオ分岐、証拠配置、難易度、エンディング条件。
- Engineer (antigravity):
  - 主編集対象: `src/karakuri/**`、ランナー接続、NodePath契約、テスト導線。
  - 責務: データ駆動実行基盤、互換性、品質担保。

## 5. 実行順（マイルストーン）
1. M1 入口統一と旧導線整理
2. M2 YAML スキーマ契約
3. M3 C++責務寄せ
4. M4 モード分離
5. M5 多言語切替（EN/JA）
6. M6 役割分離の運用固定
7. M7 受け入れテスト固定
8. M8 ドキュメント同期

詳細チェックは `TASK.md` を唯一の進捗管理表として更新する。

## 6. Definition of Done（PR単位）
- 対象タスクのチェック項目が `TASK.md` で更新されている。
- `./dev.sh run mystery` で実行確認済み。
- `godot --headless --path . --script res://samples/mystery/scripts/karakuri_scenario_smoke.gd` 実行確認済み。
- `./scripts/check_mystery_translation_keys.sh` 実行確認済み。
- 影響範囲（UI/Scenario/C++）が PR 本文に記載されている。
- 仕様差分がある場合、`docs/mystery_design.md` または本資料を同時更新している。

## 7. PR テンプレート（antigravity 用）
以下を PR 本文に貼り付ける。

```md
## Scope
- （このPRで実施したこと）

## Out of Scope
- （このPRで意図的にやらないこと）

## Changes
- [ ] UI
- [ ] Scenario(YAML)
- [ ] C++(Karakuri)
- [ ] Docs

## Test
- [ ] `./dev.sh run mystery` で起動確認
- [ ] 対象モードの遷移確認
- [ ] EN/JA 切替確認（該当時）
- [ ] HP/分岐/エンディング確認（該当時）

## Risks / Follow-up
- （既知課題、次PRへ持ち越す項目）
```

## 8. コミットメッセージ規約（推奨）
- 形式: `[Karakuri][Mystery][M{番号}] 要約`
- 例: `[Karakuri][Mystery][M5] Fix runtime locale switch in confrontation UI`

## 9. リスク一覧（先に見る）
- 旧シーンと新シーンの二重導線により、修正漏れが発生しやすい。
- 翻訳キー欠落時に英日混在表示が発生しやすい。
- NodePath変更で UI とランナー接続が壊れやすい。
- YAML 拡張時に後方互換を壊しやすい。
- `dialogue` が UI 側で待機できないと、連続テキストが上書きされやすい（`dialogue_finished` 契約）。
- `clicked_at` の座標空間が実装間で不一致だと hotspot 判定が壊れる（Canvas/World 座標で統一）。

## 10. 最小報告フォーマット（作業完了時）
- Done:
  - （完了項目）
- Pending:
  - （未完項目）
- Evidence:
  - （確認コマンド、スクリーンショット、ログ）
- Next:
  - （次PRでやること）

## 11. 関連資料
- M1チケット本文: `docs/mystery_m1_ticket_antigravity.md`
- M1レビュー観点: `docs/mystery_m1_review_checklist.md`
- UI共通ポリシー: `docs/ui_layout_policy.md`
- UIレイアウト方針: `docs/mystery_ui_layout_policy.md`
- YAML v1仕様: `docs/mystery_yaml_schema_v1.md`
- YAMLテンプレート: `samples/mystery/scenario/templates/mystery_template_v1.yaml`

## 13. 最新実装メモ（2026-02-19 / Codex）
- `KarakuriLocalizationService`（C++）を追加し、locale 永続化と runtime 切替を一本化。
  - `src/karakuri/karakuri_localization_service.h`
  - `src/karakuri/karakuri_localization_service.cpp`
  - `samples/common/karakuri_localization_service.tscn`（autoload）
- `KarakuriScenarioRunner` に mode 共通インターフェース呼び出しを追加。
  - `on_mode_enter(mode_id, scene_id)`
  - `on_mode_exit(mode_id, next_scene_id)`
  - `set_mode_input_enabled(enabled)`
- `testimony` の進行制御（正誤判定/HP/成功失敗分岐）を C++ 側へ移管。
- `samples/mystery/scenario/mystery.yaml` を `text_key/speaker_key/shake_key` ベースへ移行。
- 翻訳キー未登録チェック手順をスクリプト化。
  - `scripts/check_mystery_translation_keys.sh`
- 受け入れスモークを更新。
  - `samples/mystery/scripts/karakuri_scenario_smoke.gd`

## 12. M1レビュー指摘事項（2026-02-17）
M1完了報告に対してレビューを実施した時点では **M1完了判定は保留**。

### 重大（先に修正）
- `choice` のシグナル引数契約不一致
  - 現象: `res://samples/mystery/scripts/dialogue_ui_advanced.gd` 側の `choice_selected(index, text)` 契約とランナー契約が一致していないと分岐が壊れる。
  - 影響: 選択肢分岐でコールバック不整合が起きる可能性。
  - 該当: `samples/mystery/karakuri_mystery_shell.tscn`, `samples/mystery/scripts/dialogue_ui_advanced.gd`, `src/karakuri/scenario/karakuri_scenario_runner.cpp`

- `testimony` 必須APIの欠落
  - 現象: `res://samples/mystery/scripts/testimony_system.gd` が `add_testimony` / `start_testimony` / `all_rounds_complete` を満たさないと対決が起動できない。
  - 影響: YAMLの `testimony` アクション実行時にランナー側要件を満たせず進行停止。
  - 該当: `samples/mystery/karakuri_mystery_shell.tscn`, `samples/mystery/scripts/testimony_system.gd`, `src/karakuri/scenario/karakuri_scenario_runner.cpp`

- InventoryUIがランナー契約未達
  - 現象: InventoryUI が `add_evidence/show_inventory` 契約を満たさないと証拠追加/提示フローが壊れる。
  - 該当: `samples/mystery/karakuri_mystery_shell.tscn`, `samples/mystery/ui/evidence_inventory_ui.tscn`, `samples/mystery/scripts/inventory_ui.gd`

### 中（M1範囲管理）
- M1スコープ外変更の混入
  - 現象: `samples/mystery/scenario/mystery.yaml` にシーン構成・進行の大幅変更を同梱。
  - 影響: M1（入口統一）のレビュー粒度が崩れ、切り分け困難。
  - 方針: M1とシナリオ拡張をPR分離する。

- ドキュメント未同期
  - 現象: `docs/mystery_design.md` に削除済み `main_mystery.tscn` 記載が残存。生成スクリプト `scripts/create_main_mystery.gd` も旧パスを出力先に保持。
  - 影響: 実装と手順の不一致。
  - 方針: M1完了前に記述更新。

### M1クローズ条件（再確認）
- `./dev.sh run mystery` で正式導線のみで起動し、開始から進行可能であること。
- 旧導線（`main_mystery.tscn`）の扱いがコード・ドキュメント双方で明示されていること。
- PR本文に `Scope / Out of Scope / Test / Risks` があり、M1外変更は分離されていること。

### 対応状況（2026-02-18 / Codex）
- [x] `karakuri_mystery_shell.tscn` をランナー契約に合う UI スクリプト構成へ修正
- [x] `main_mystery.tscn` 削除後の旧参照をドキュメント/生成スクリプトから整理
- [x] `TASK.md` の M1 項目を完了化
- [ ] M1スコープ外で混在した `samples/mystery/scenario/mystery.yaml` の扱い分離（別PR推奨）

## 14. 設計評価レポート（2026-02-19 / テンプレート目的との整合チェック）

本セクションは「ユニバーサル・ゲームテンプレート」プロジェクト（`docs/design_doc.md`）の目的に対し、
現行 Mystery 実装の設計品質を評価した結果をまとめる。

プロジェクト目標:
- **View × Style × Cycle の組み合わせ可能な再利用基盤**を C++ コアと Godot シーンの完全分離で実現する。
- Mystery デモ = Adventure/Novel スタイル + Story サイクルの **参照実装** として機能させる。

---

### 14.1 ドキュメント不整合（即時修正推奨） ✅ **対応済み**

`docs/mystery_design.md § 5.1 NodePath 契約表` の TestimonySystem 行が **M3 以前の旧 API** を記載したまま。

| 項目 | ドキュメント記載（旧） | 現行実装（M3後） |
|---|---|---|
| TestimonySystem 必須 API | `add_testimony`, `start_testimony`, `all_rounds_complete` | `next_requested`(signal), `shake_requested`(signal), `present_requested`(signal) を C++ が受信。GDScript 側は UI 表示のみ。 |

**対応**: `docs/mystery_design.md § 5.1` の TestimonySystem 行を現行実装（signal 受信契約）に更新する。

---

### 14.2 KarakuriScenarioRunner のモノリシック化（テンプレート再利用性の最大リスク） ✅ **対応済み**

現在の `karakuri_scenario_runner.cpp` は 1,237 行に達し、以下を単一クラスで処理している。

- YAML 解析・Scene ロード
- Action 実行エンジン（dialogue / goto / choice / if_flag / testimony 等）
- Hotspot 判定
- Testimony 進行制御（ラウンド管理・正誤・HP減少・終了）
- TranslationServer 呼び出し（`tr_key`）

`design_doc.md` では `StorySession / RoguelikeSession / SandboxSession` の切替を想定しているが、
現行の Runner は Mystery 固有処理（testimony / damage 判定）が Action 層に埋め込まれており、
**他のゲームスタイルへの流用時に不要な依存が残る**。

将来リスク:
- 新サイクル（Roguelike, Sandbox）追加時に Runner を丸ごとコピーして改造するパターンに陥りやすい。
- Runner を分割しようとしたとき `testimony_lines_` 等のステートが Action 進行ステートと混在しているため分離しづらい。

**推奨方針（次期対応）**:
1. Runner 内の Testimony 進行ステート（`testimony_lines_`, `testimony_index_`, `testimony_round_` 等）を
   別クラス `KarakuriTestimonySession` または `KarakuriConfrontationState` に切り出す。
2. Action 実行エンジン部分（`execute_single_action`, `step_actions`）を `KarakuriActionQueue` として分離し、
   Story 固有でない pure なキューとして再利用できる形にする。
3. `Mystery 固有 action`（testimony / take_damage）と `汎用 action`（dialogue / goto / if_flag）を
   API コメントで明示的に区別する。

**対応内容**（実施済み）:
- `src/karakuri/scenario/karakuri_testimony_session.h` を新規作成。
  `KarakuriTestimonySession` struct（`Line` 内包型、全 testimony フィールド、`reset()` メソッド）を定義。
- Runner ヘッダの `struct TestimonyLine` とフラットフィールド群
  （`testimony_lines_`, `testimony_index_`, `testimony_active_`, `waiting_for_testimony_` 等 9 フィールド）を削除。
  `KarakuriTestimonySession testimony_{}` 1 つに置き換え。
- Runner.cpp のすべての参照（約 53 箇所）を `testimony_.active` / `testimony_.index` 等に置換。
  `complete_testimony()` ではアクション取り出しを `reset()` 前に行うよう順序を整理。
- `register_mystery_actions()` で Mystery 固有アクション（testimony / take_damage 等）が分離済みのため
  Action 登録レイヤーの境界は明示されている（上記 3. 完了）。

---

### 14.3 TASK.md Phase 5 との責務重複リスク ✅ **対応済み**

TASK.md Phase 5「Core Adventure Systems」には以下が未実装として残っている。

- `GlobalState` (Autoload): フラグ・変数・ストーリー進行管理
- `SceneTransitionManager`: フェード遷移
- `ChoiceManager`: 選択肢表示・分岐ロジック
- `FlagCondition`: フラグ条件付き表示

一方、現行 `KarakuriScenarioRunner` はこれらの責務をすでに内包している
（`set_flag`, `if_flag`, `choice`, `goto`, `scenes` ロード）。

Phase 5 を別途実装すると **Runner との二重管理** が生じる。

**推奨方針**:
- Phase 5 は `KarakuriScenarioRunner` の既存実装を「参照実装」と位置付け、
  フォーマルな Autoload クラスとして **Karakuri 側に昇格させる形** で実装する。
- `GlobalState` → `AdventureGameStateBase`（現行 C++）で代替可能かを先に評価し、
  GDScript 側に重複実装しない。

**対応内容**（実施済み）:
- `TASK.md Phase 5` の "Core Adventure Systems" / "Court/Confrontation Mode" に代替実装テーブルと `[x]` マークを追加。
- `SceneTransitionManager`（フェード遷移）と各種 Evidence/Character/Investigation システムのみ未実装として残すよう整理。

---

### 14.4 ゆさぶり（Shake）実行中の dialogue 待機なし（前回レビュー持ち越し） ✅ **対応済み**

`on_testimony_shake_requested()` 内で `dialogue_ui_->call("show_message_with_keys", ...)` を呼ぶが、
`waiting_for_dialogue_ = true` を設定していない。

証言モードは `waiting_for_testimony_ = true` でアクション進行自体をブロックしているため
現状は動作するが、以下のリスクがある。

- ゆさぶり台詞表示中にプレイヤーが Next/Present ボタンをすぐ押せる（入力競合）。
- 将来ゆさぶり応答に `dialogue_finished` 待機が必要な演出を追加した場合、無音で壊れる。

**対応案（2択）**:
1. 意図通り「即時押し可」とするなら、`on_testimony_shake_requested` 内に
   `// Intentional: shake feedback does not block testimony input` のコメントを追加して明示。
2. dialogue 待機を持たせる場合は `waiting_for_dialogue_ = true` + `waiting_for_testimony_` との
   両方を立てるか、shake 専用の waiting フラグを用意する。

---

### 14.5 hotspot_matches_click のスケール非考慮（前回レビュー持ち越し） ✅ **対応済み**

`hotspot_matches_click()` は `Area2D::get_global_position()` と `RectangleShape2D::get_size()` の組み合わせで判定しているが、
`Area2D` またはその祖先ノードに非 1.0 スケールが掛かる場合にヒット判定がずれる。

現行サンプルシーンはスケール=1 のため実害なし。ただしシーンを再利用・流用した際に無音で壊れやすい。

**推奨**: `area->get_global_transform()` を用いた AABB 判定、または
`RectangleShape2D` の場合は `Transform2D::xform()` でサイズをグローバル座標に変換する。

---

### 14.6 セーブ/ロード未実装によるプレイアブル完成度の欠如 ✅ **対応済み**

`basic_game_karakuri.md` のスコープには `SaveData（最小のセーブ/ロード）` が明記されているが未実装。
現状、YAML フラグ・インベントリ・HP はすべてメモリ上のみに存在し、
**ゲームを中断・再起動すると進行が失われる**。

テンプレート参照実装として他開発者が Mystery デモを参照する際に
「セーブなしのゲームがデフォルト」という誤解を生みやすい。

**対応内容**（実装済み）:
- `KarakuriSaveService`（`src/karakuri/karakuri_save_service.h/cpp`）を新規追加。
  `AdventureGameStateBase` の `flags`/`inventory`/`health` を
  `user://karakuri/<demo_id>/save.json` に JSON 形式で永続化。
- `AdventureGameStateBase` に `get_flags_snapshot()` / `restore_flags_snapshot()` /
  `get_inventory_snapshot()` / `restore_inventory_snapshot()` を追加（スナップショット API）。
- `KarakuriScenarioRunner::init_builtin_actions()` に `save` / `load` アクションを登録。
  YAML から `- save: mystery` / `- load: mystery` で呼び出し可能。
- GDScript から `KarakuriSaveService.save_game("mystery")` などの静的メソッドで直接呼び出し可能。

---

### 14.7 YAML テンプレートの具体化による転用障壁 ✅ **対応済み**

`samples/mystery/scenario/templates/mystery_template_v1.yaml` が都市伝説テーマの具体的なコメントや
固有 `speaker_key`（`speaker.detective`, `speaker.rat_witness` 等）を含んでいる。

他ジャンルへ転用するプランナーがテンプレートをコピーした際、
Mystery 固有の名称を置き換える作業負荷が高い。

**推奨**:
- テンプレートは `speaker_key: "speaker.protagonist"` / `text_key: "scene001.line001"` のような
  **ジャンル非依存のプレースホルダー** に変更する。
- Mystery 固有のサンプルは `templates/mystery_sample_v1.yaml` として別ファイルに分離する。
