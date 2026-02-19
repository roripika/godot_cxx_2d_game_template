# Tasks: Universal Game Template

## Project Maintenance
- [x] Create/Move Agent Rules to `.agent/AGENTS.md`

## アーキテクチャ決定・ドキュメント整備（今回の実装レビューから抽出）

### 今すぐ決める（次PR前に必要）

- [x] **①** `KarakuriScenarioRunner` の置き場を正式決定し `AGENTS.md § 3` と `docs/basic_game_karakuri.md` に明記する
  - ~~A案: Runnerは `samples/mystery/` に移す（汎用ActionキューだけKarakuriに残す）~~
  - **B案採用（2026-02-20）**: Runnerは `src/karakuri/` に置く。Mystery固有Action（`testimony`/`take_damage`/`if_health_ge`/`if_health_leq`）は `register_mystery_actions()` として公開し、Mystery shell `_ready()` から注入する。

- [x] **②** C++バインディング変更時のGDScript更新ルールを `AGENTS.md § 3` に追記する
  - 「公開APIを変えたら同一コミットで接続先GDScriptを全て更新する」規約を明文化する（2026-02-20 完了）

- [x] **③** `user://` ファイル命名規約を `docs/basic_game_karakuri.md` に追記する
  - `user://karakuri/<filename>`（共通）・`user://karakuri/<demo_id>/<filename>`（デモ固有）を規約化（2026-02-20 完了）
  - `KarakuriLocalizationService` のパスを `user://karakuri_locale.txt` → `user://karakuri/locale.txt` に変更済み

### 近いうちに文書化する

- [x] **④** GDScriptに書いてよい処理の「正リスト」を `AGENTS.md § 2` に追記する
  - 「ViewはGDScript、ロジックはC++」をテーブル例付きで具体化（2026-02-20 完了）

- [x] **⑤** `mode_id`（investigation / deduction / confrontation / ending）がMystery専用か全Adventure共通語彙かを `docs/mystery_yaml_schema_v1.md` に明記する
  - Mystery固有語彙だが、他デモが任意文字列を使うことは可能と明記（2026-02-20 完了）

- [x] **⑥** autoload の命名・登録スコープルールを `docs/demo_plan.md § 共通Utility` に追記する
  - Karakuri共通はグローバル登録OK、デモ固有は将来シーン内ノード化の方針を記載（2026-02-20 完了）

### 将来設計として固めておく

- [x] **⑦** YAML スキーマのバージョンアップポリシー（後方互換の保証範囲）を `docs/mystery_yaml_schema_v1.md` 末尾に追記する
  - 後方互換テーブル・v2 移行ルール・`schema_version` フィールド導入方針を記載（2026-02-20 完了）

- [x] **⑧** スモークテスト（`karakuri_scenario_smoke.gd`）のCI化方針を `AGENTS.md` に追記する
  - C++ 変更を含む PR は必須、それ以外は任意推奨。CI化時は `./dev.sh smoke` をエントリポイントとする方針を記載（2026-02-20 完了）

---

## Priority: Mystery Demo Execution (Owner: antigravity)

### 運用ルール（固定）
- [ ] 作業者は antigravity に固定し、PR本文に `Scope / Out of Scope / 検証結果` を必ず記載する
- [x] `src/karakuri/**` の C++ は公開 API に Doxygen コメントを必須化する（commit: `0403f7f`）
- [ ] Basic Game Karakuri にデモ固有ロジックを入れない（依存は `samples -> karakuri` のみ）

### 現状確認（2026-02-19）
- [x] ミステリーの起動入口は `res://samples/mystery/karakuri_mystery_shell.tscn`
- [x] `KarakuriScenarioRunner` は `samples/mystery/scenario/mystery.yaml` を読む構成
- [x] 推理・対決の進行データは YAML 側に追加済み（commit: `bcaa936`）
- [x] `dialogue` は `dialogue_finished` 契約で待機する（commit: `31a6cfe`）
- [x] `clicked_at` は Canvas/World 座標契約に統一（commit: `1acb0c2`）
- [x] Legacy 重複 UI 実装を整理し、`samples/mystery/ui/*.tscn` を正式化（commit: `1aeefb7`）
- [x] `docs/mystery_design.md` を現行実装契約へ同期（commit: `bbc7a82`）

### M1: 入口統一と旧導線の整理
- [x] 正式導線を `karakuri_mystery_shell.tscn` に一本化し、旧導線の扱い（削除/互換）を明記する
- [x] `samples/mystery/main_mystery.tscn` など旧シーン参照の棚卸しを実施する
- [x] `README.md` と `docs/mystery_design.md` の起動手順を一本化する

### M2: YAML スキーマ契約（Planner編集用）
- [x] `docs/` に YAML v1 仕様（モード、分岐、証拠、対決、終了条件）を明文化する（`docs/mystery_yaml_schema_v1.md`）
- [x] 必須キー不足時のエラーポリシー（ログ/停止/フォールバック）を決める（`docs/mystery_yaml_schema_v1.md`）
- [x] サンプルテンプレート YAML（最小1本）を用意し、プランナーがコピーして増やせる状態にする（`samples/mystery/scenario/templates/mystery_template_v1.yaml`）

### M3: C++責務寄せ（Basic Game Karakuri）
- [x] YAML 読込・状態遷移・フラグ/所持品/HP の正を C++ 側に固定する（commit: `54540d8`, `ff79e74`）
- [x] GDScript は UI 表示と入力受け取りに限定する（commit: `0403f7f`）
- [x] 文章直書きを削減し、翻訳キーまたはデータ参照に統一する（commit: `0403f7f`）

### M4: モード分離（Investigation / Deduction / Confrontation / Ending）
- [x] 各モードの共通インターフェース（入力、UI更新、遷移）を定義する（commit: `0403f7f`）
- [x] モード遷移を YAML 記述のみで制御できるようにする（commit: `54540d8`）
- [x] 失敗時（HP 0）と成功時（複数エンディング）の分岐を固定する（commit: `a350204`）

### M5: 多言語切替（EN/JA）再実装
- [x] 実行中の言語切替で「会話・選択肢・インベントリ・ボタン」が即時更新される（commit: `0403f7f`）
- [x] 起動時 locale 復元（永続化）が動作する（commit: `0403f7f`）
- [x] ミステリー用翻訳キーの未登録チェック手順を定義する（commit: `0403f7f`）

### M6: 役割分離（Designer / Planner）
- [x] Designer の編集対象を `samples/mystery/ui/**` と `tscn/theme` に限定する運用を文書化する（commit: `bbc7a82`）
- [x] Planner の編集対象を `samples/mystery/scenario/**` と翻訳データに限定する運用を文書化する（commit: `bbc7a82`）
- [x] ノード名契約（UI差し替え時に壊してはいけない NodePath）を明記する（commit: `bbc7a82`）

### M7: 受け入れテスト固定
- [x] `godot --headless --path . --script res://samples/mystery/scripts/karakuri_scenario_smoke.gd` で開始からエンディングまで到達する（good/bad 両分岐）
- [x] 証拠提示の正解/不正解、ゆさぶり、HP減少、ゲームオーバーを確認する（`karakuri_scenario_smoke.gd`）
- [x] 実行中の EN/JA 切替を各モードで確認する（`karakuri_scenario_smoke.gd`）
- [x] PRごとに手動テスト結果をチェックリストで添付する（テンプレート定義: `docs/mystery_antigravity_handover.md`）
- [x] ヘッドレスの最低動作確認を固定する（`samples/mystery/scripts/karakuri_scenario_smoke.gd`, latest pass: `0403f7f`）

### M8: ドキュメント同期
- [x] `TASK.md`、`docs/mystery_design.md`、`README.md`、引き継ぎ資料を同一内容に同期する（commit: `0403f7f`）
- [x] 完了済み項目はコミットID付きで `[x]` に更新する
- [x] 未完項目は次PRの先頭タスクに繰り越す（繰り越し: 運用ルール 2項目）

### 参照
- [x] 引き継ぎ資料 `docs/mystery_antigravity_handover.md` を更新し続ける（commit: `0403f7f`）

## Phase 1: Core Architecture (Logic/View Separation)
- [x] **Refactor Project Structure**
    - [x] Create `src/core`, `src/views`, `src/entities` directories
    - [x] Update `SConstruct` to compile sources from new directories
- [x] **Core Data Layer**
    - [x] Implement `UniversalWorldData` (Resource)
        - [x] 3D Voxel storage (flat vector or map)
        - [x] Signal emission on data change (`voxel_changed`)
    - [ ] Implement `GameCycleManager` (Node)
        - [ ] State Machine for Cycle (Roguelike/Story/Sandbox)
- [x] **View Layer Adapters**
    - [x] Implement `IsometricView` (inherits `TileMapLayer`)
        - [x] Connect to `UniversalWorldData`
        - [x] Render 2D Isometric tiles based on 3D data (z-slicing or projection)
    - [x] **[NEW]** Implement `SideScrollingView` (inherits `TileMapLayer`)
        - [x] Connect to `UniversalWorldData`
        - [x] Render Side view (x, y) from 3D data (z=depth)

## Phase 2: Game Style & Entities
- [x] **Entity System**
    - [x] Implement `GameEntity` (CharacterBody2D/3D wrapper)
        - [x] Stats logic (HP, Speed)
        - [x] Movement input Interface
- [x] **Player Controllers**
    - [x] `PlayerControllerIso` (Isometric movement)
    - [x] `PlayerControllerSide` (Platformer movement)
    - [ ] `PlayerControllerFPS` (First-person movement)

## Phase 2.5: Items & Inventory (Hack & Slash Foundation)
- [x] **Item Data Structure**
    - [x] `GameItem` (Resource): Base class (Name, Icon, Type)
    - [ ] `EquipmentItem` (Inherits `GameItem`): Stats (Attack, Defense)
- [x] **Inventory System**
    - [x] `Inventory` (Node/Resource): List of items, Add/Remove logic
- [ ] **Loot System**
    - [ ] `LootTable`: Weighted random item generation

## Phase 3: Game Cycles (Roguelike Loop)
- [ ] **Cycle Implementation**
    - [x] `RoguelikeGenerator` (Procedural Map):
        - [x] Room & Corridor Generation Algorithm
        - [x] Apply to `UniversalWorldData` (Floor=0, Wall=1)
    - [x] `RoguelikeManager` (Node):
        - [x] Orchestrate Dungeon Generation
        - [x] Spawn Player at Start Point
    - [ ] `SandboxSession`: Infinite generating & Saving

## Phase 4: Sample & Demo System (Templates)
- [x] **Sample Organization**
    - [x] Create `samples/` directory in Godot project.
    - [x] **Demo 1: Roguelike**: `samples/roguelike/` (Dungeon Crawler)
    - [x] **Demo 2: Platformer**: `samples/platformer/` (Side Action)
    - [x] **Demo 3: Mystery**: `samples/mystery/` (Urban Myth Adventure)
    - [x] **Demo 4: Sandbox**: `samples/sandbox/` (CoreKeeper-like Mining/Crafting)
    - [x] **Demo 5: Fighting**: `samples/fighting/` (Street Fighter-like VS Battle)
- [x] **Setup Tool**
    - [x] `scripts/setup_demo.sh`: Script to switch `project.godot` main scene to selected demo.

## Phase 5: Adventure Features (逆転裁判/都市伝説スタイル)

> **責務整理ノート（2026-02-20）**: 下記 "Core Adventure Systems" の多くは
> `KarakuriScenarioRunner` + `AdventureGameStateBase` で実装済み。
> 別途 GDScript クラスを作ると二重管理になるため、以下の代替方針を採用する。
>
> | Phase 5 項目 | 代替実装（実装済み） |
> |---|---|
> | `GlobalState` (Autoload) | `AdventureGameStateBase`（C++ singleton, flags/inventory/health） |
> | `ChoiceManager` | `KarakuriScenarioRunner` の `choice` アクション |
> | `FlagCondition` | `KarakuriScenarioRunner` の `if_flag` / `if_has_items` アクション |
> | `TestimonySystem` | `KarakuriScenarioRunner` の `testimony` アクション + GDScript UI |
> | `ContradictionDetector` | `testimony` アクション内の `present` signal 処理 |
> | `HealthBar` | `AdventureGameStateBase::health` + `health_changed` signal |
> | `SaveData` | `KarakuriSaveService`（C++, `save` / `load` YAML アクション） |
>
> 残り未実装の項目のみ別途実装すること。

- [x] **Interaction System**
    - [x] `InteractionManager`: Mouse Input & Signal emission.
    - [x] `DialogueUI`: Text display components.
- [ ] **Core Adventure Systems**
    - [x] `GlobalState` → `AdventureGameStateBase`（C++）で代替済み
    - [ ] `SceneTransitionManager`: Seamless scene switching with fade effects.
    - [x] `ChoiceManager` → `KarakuriScenarioRunner::choice` アクションで代替済み
    - [x] `FlagCondition` → `KarakuriScenarioRunner::if_flag` アクションで代替済み
- [ ] **Evidence & Inventory**
    - [ ] `EvidenceItem` (Resource): Evidence data (Name, Description, Icon, Category).
    - [ ] `EvidenceManager`: Add/Remove/Check evidence.
    - [ ] `InventoryUI`: Display evidence list with details view.
    - [ ] `EvidencePresentSystem`: Click evidence to present in dialogue.
- [ ] **Character & Presentation**
    - [ ] `CharacterPortrait`: Sprite with multiple expressions/poses.
    - [ ] `PortraitManager`: Switch portraits based on dialogue.
    - [ ] `TextAnimator`: Type-writer effect with speed control.
- [ ] **Investigation Mode**
    - [ ] `InvestigationScene`: Click hotspots to examine/talk/move.
    - [ ] `HotspotManager`: Define clickable areas with descriptions.
    - [ ] `LocationMap`: Visual map for scene navigation.
- [ ] **Court/Confrontation Mode**
    - [x] `TestimonySystem` → `KarakuriScenarioRunner::testimony` アクションで代替済み
    - [x] `ContradictionDetector` → testimony 内 `present` signal 処理で代替済み
    - [x] `HealthBar` penalty → `AdventureGameStateBase::health` で代替済み
- [ ] **Demo Scenes**
    - [ ] **Scene 1: Investigation**: Office with clues and NPCs.
    - [ ] **Scene 2: Dialogue**: Conversation with choices.
    - [ ] **Scene 3: Presentation**: Court-style evidence presentation.

    - [ ] **Scene 4: Multiple Endings**: Flag-based story branches.

## Phase 6: Fighting Game Features [NEW]
- [ ] **Combat System**
    - [ ] `FightingEntity` (Inherits GameEntity): State Machine (Idle, Walk, Attack, Stun).
    - [ ] `HitboxManager`: Frame-based collision detection.
    - [ ] `InputBuffer`: Command interpretation (Quarter-circle, etc.).

## Phase 7: Sandbox Features [NEW]
- [ ] **World Interaction**
    - [ ] `MiningSystem`: Break voxels and drop items.
    - [ ] `BuildingSystem`: Place voxels from inventory.

## Phase 8: Shooting Game Features
- [ ] **Bullet System**
    - [ ] `BulletManager`: Object pooling for projectiles.
    - [ ] `BulletPattern`: Danmaku pattern generator.
- [ ] **Enemy Formation**
    - [ ] `EnemyWaveManager`: Spawn enemies in waves.
    - [ ] `PathFollower`: Predefined enemy movement paths.

## Phase 9: Puzzle Game Features
- [ ] **Board System**
    - [ ] `PuzzleBoard`: Grid-based tile matching.
    - [ ] `MatchDetector`: Pattern recognition (match-3, etc.).
- [ ] **Physics Puzzle**
    - [ ] `FallingBlockSystem`: Gravity-based tile fall.

## Phase 10: Strategy Game Features
- [ ] **Unit Management**
    - [ ] `RTSUnitController`: Multi-unit selection & command.
    - [ ] `PathfindingManager`: A* / Flow field navigation.
- [ ] **Turn-Based System**
    - [ ] `TurnManager`: Initiative-based turn order.
    - [ ] `GridTacticsController`: SRPG-style movement.

## Phase 11: Card Game Features
- [ ] **Card System**
    - [ ] `Card` (Resource): Base card data (Cost, Effect).
    - [ ] `Deck`: Card collection with shuffle/draw.
    - [ ] `Hand`: Active cards display & management.
- [ ] **Battle System**
    - [ ] `CardBattleManager`: Turn flow for card battles.

## Phase 12: Tower Defense Features
- [ ] **Tower System**
    - [ ] `Tower`: Turret with range & targeting.
    - [ ] `TowerPlacement`: Grid-based tower building.
- [ ] **Enemy Waves**
    - [ ] `WaveSpawner`: Timed enemy generation.
    - [ ] `PathManager`: Enemy movement along predefined routes.

## Phase 13: Racing Game Features
- [ ] **Vehicle System**
    - [ ] `VehicleController`: Acceleration, steering, drift.
    - [ ] `TrackManager`: Checkpoint & lap system.
- [ ] **Physics**
    - [ ] Custom vehicle physics or Godot VehicleBody3D integration.

## Phase 14: Simulation Features
- [ ] **Resource Management**
    - [ ] `ResourcePool`: Complex economy system (money, materials, etc.).
    - [ ] `ProductionChain`: Resource conversion & crafting trees.
- [ ] **AI Characters**
    - [ ] `NPCScheduler`: Daily routine & behavior trees.
    - [ ] `RelationshipSystem`: NPC affinity & reaction.

## Phase 15: Metroidvania Features
- [ ] **Exploration System**
    - [ ] `MapRevealSystem`: Fog of war & area discovery.
    - [ ] `AbilityGate`: Block areas until player gains abilities.
- [ ] **Progression**
    - [ ] `PowerupManager`: Collectible abilities (double jump, etc.).

## Legacy Tasks (Completed)
- [x] Initial Isometric Sandbox Setup (Godot 4.3 + GDExtension)
- [x] Environment Setup (macOS M2)
