# Mystery Demo 設計ドキュメント（現行実装準拠）

## 1. 目的
- Mystery デモを「再利用可能なアドベンチャー基盤」の参照実装として維持する。
- プランナーは YAML、デザイナーは UI シーン、エンジニアは C++ ランタイムと接続契約を担当する。
- 本ドキュメントは実装の一次情報とし、差分が出たら本書を更新する。

## 2. 現在の正式導線
- 実行入口: `project.godot` の `run/main_scene`
- メインシーン: `res://samples/mystery/karakuri_mystery_shell.tscn`
- シナリオ実行: `KarakuriScenarioRunner`（`src/karakuri/scenario/karakuri_scenario_runner.cpp`）
- シナリオデータ: `samples/mystery/scenario/mystery.yaml`

## 3. アーキテクチャ概要
### 3.1 構成
- Core State: `AdventureGameStateBase`（C++ / Autoload）
- Scenario Runtime: `KarakuriScenarioRunner`（C++）
- View/UI: `samples/mystery/karakuri_mystery_shell.tscn` と `samples/mystery/ui/*.tscn`
- Planner Data: `samples/mystery/scenario/*.yaml`

### 3.2 処理フロー
1. `KarakuriScenarioRunner` が YAML をロード。
2. `start_scene` の `scene_path` を `SceneContainer` 配下にインスタンス化。
3. `on_enter` アクション配列を順次実行。
4. `hotspots` の `node_id` とクリック入力を照合し、`on_click` アクションを実行。
5. `goto` / `choice` / `testimony` で進行を分岐。
6. `testimony` の正誤判定・HP減少・成功/失敗分岐は C++ ランナー側で処理する（UI は入力と表示のみ）。

## 4. 役割分担（運用契約）
### 4.1 Designer
- 主編集対象: `samples/mystery/ui/**`, `samples/mystery/karakuri_mystery_shell.tscn`
- 責務: レイアウト、視認性、アンカー、CanvasLayer 優先順位
- 制約: Node 名や NodePath 契約を壊さない

### 4.2 Planner
- 主編集対象: `samples/mystery/scenario/**`, 翻訳データ
- 責務: 進行分岐、台詞、証拠配置、失敗条件
- 制約: YAML v1 契約を守る（`docs/mystery_yaml_schema_v1.md`）

### 4.3 Engineer
- 主編集対象: `src/karakuri/**`, `src/core/**`, 接続用 GDScript
- 責務: 実行ランタイム、座標契約、UI API 契約、互換性維持
- 制約: `src/karakuri/**` の公開 API は Doxygen コメント必須

## 5. 実行時契約
### 5.1 NodePath 契約（Shell）
対象: `samples/mystery/karakuri_mystery_shell.tscn`

| 用途 | NodePath | 要件 |
|---|---|---|
| SceneContainer | `../SceneContainer` | Base scene の差し替え先 |
| Dialogue UI | `../MainInfoUiLayer/DialogueUI` | `show_message`, `show_choices`, `choice_selected`, `dialogue_finished` |
| Evidence UI | `../InstantSubInfoUiLayer/InventoryUI` | `add_evidence`, `show_inventory`（任意） |
| InteractionManager | `../InteractionManager` | `clicked_at(Vector2)` signal |
| TestimonySystem | `../MainInfoUiLayer/TestimonySystem` | `add_testimony`, `start_testimony`, `all_rounds_complete` |

### 5.2 クリック座標契約
- `InteractionManager.clicked_at(position)` の `position` は Canvas/World 座標。
- Hotspot 判定は `Area2D.global_position` と同一座標空間で比較する。
- 参照: `src/core/interaction_manager.cpp`, `src/karakuri/scenario/karakuri_scenario_runner.cpp`

### 5.3 dialogue 待機契約
- `dialogue` action 実行時、UI が `dialogue_finished` を提供する場合は signal 発火まで次 action に進まない。
- UI が signal を持たない場合は待機せず進行する。

### 5.4 モード共通インターフェース契約
対象 UI ノードが以下メソッドを実装する場合、ランナーは mode 遷移時に統一呼び出しする。

| メソッド | 呼び出しタイミング | 目的 |
|---|---|---|
| `on_mode_enter(mode_id, scene_id)` | scene 読込後 | モード開始時の UI 初期化 |
| `on_mode_exit(mode_id, next_scene_id)` | scene 切替前 | モード終了時の後始末 |
| `set_mode_input_enabled(enabled)` | action 実行中/待機中の切替時 | 入力可否の統一制御 |

`mode_id` は YAML の `scene.mode`（`investigation / deduction / confrontation / ending`）を優先し、未指定時は `scene_id` から推定する。

## 6. ゲームモード構成（現行 YAML）
現状 `samples/mystery/scenario/mystery.yaml` では以下の scene_id で構成する。
- `prologue`
- `warehouse_investigation`
- `deduction`
- `confrontation`
- `ending_good`
- `ending_bad`

備考:
- 現行データはエンディング 2 系統（good/bad）。
- `take_damage` は HP ストライク制（`AdventureGameStateBase.health`）に合わせて 1 単位で運用。

## 7. YAML 設計方針
- 仕様書: `docs/mystery_yaml_schema_v1.md`
- テンプレート: `samples/mystery/scenario/templates/mystery_template_v1.yaml`
- action は 1 キー辞書で記述する。
- Mystery 本運用では `text_key / speaker_key / shake_key` を使用し、`text` は互換フォールバックとしてのみ許容する。
- `scene_id`, `flag`, `item` は `snake_case` を推奨。

## 8. UI 設計方針
- 共通ポリシー: `docs/ui_layout_policy.md`
- Mystery 拡張: `docs/mystery_ui_layout_policy.md`
- レイヤー実装値（現行）:
  - `SystemUiLayer.layer = 30`
  - `InstantSubInfoUiLayer.layer = 20`
  - `MainInfoUiLayer.layer = 10`
  - `SubInfoUiLayer.layer = 0`

## 9. ディレクトリ責務（現行）
```text
samples/mystery/
├── karakuri_mystery_shell.tscn          # 正式エントリ
├── scenario/mystery.yaml                # Planner が編集する進行データ
├── ui/evidence_inventory_ui.tscn        # Inventory UI（デザイナー主担当）
├── ui/testimony_ui.tscn                 # Testimony UI（デザイナー主担当）
├── scripts/dialogue_ui_advanced.gd      # 会話 UI ロジック
├── scripts/inventory_ui.gd              # 証拠 UI ロジック
├── scripts/testimony_system.gd          # 対決 UI ロジック
└── data/evidence/*.tres                 # 証拠マスタ
```

## 10. 受け入れチェック（最低限）
- `./dev.sh run mystery` で起動できる。
- `godot --headless --path . --script res://samples/mystery/scripts/karakuri_scenario_smoke.gd` が成功する。
- `./scripts/check_mystery_translation_keys.sh` が成功する。
- 証拠取得/選択肢/対決/エンディング遷移が破綻しない。
- UI 変更時に `docs/mystery_ui_layout_policy.md` の台帳を更新する。

## 11. 既知ギャップ（次期対応）
- 旧サンプル（`office_scene.tscn` など）は参照用途として残存しており、正式導線は shell 固定で運用する。

## 12. 関連資料
- `docs/demo_plan.md`
- `docs/mystery_antigravity_handover.md`
- `docs/mystery_yaml_schema_v1.md`
- `docs/mystery_ui_layout_policy.md`
- `TASK.md`
