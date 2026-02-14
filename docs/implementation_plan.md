# ユニバーサル・ゲームテンプレート 実装計画

## 目標
特定の「アイソメトリック・サンドボックス」プロジェクトを、以下を扱える **モジュラー型ユニバーサル・ゲームテンプレート** に変換します。
- **View (視点)**: 2D Iso、2D Side、3D、FPS
- **Style (スタイル)**: RPG、アクション、アドベンチャー、格闘、サンドボックス
- **Cycle (サイクル)**: ローグライク、サンドボックス、ストーリー、対戦

## アーキテクチャ

### 1-5. コア、ビュー、エンティティ、アイテム、サイクル (完了/進行中)
- `UniversalWorldData`、`IsometricView`、`SideScrollingView`
- `GameEntity`、`PlayerControllerIso`、`PlayerControllerSide`
- `GameItem`、`Inventory`
- `RoguelikeGenerator`、`RoguelikeManager`

### 6. サンプル & デモシステム (Phase 4)
- **ディレクトリ構成**:
    - `godot_project/samples/`
        - `roguelike/`: プロシージャル・ダンジョン
        - `platformer/`: 横スクロールアクション
        - `mystery/`: アドベンチャー / ビジュアルノベル
        - `sandbox/`: 採掘 & クラフト (CoreKeeperスタイル)
        - `fighting/`: 1v1格闘 (ストリートファイタースタイル)

### 7. 機能拡張

#### アドベンチャー (Phase 5) - 逆転裁判/都市伝説スタイル
- **`InteractionManager`**: マウス/グリッドインタラクション (完了)。
- **`DialogueUI`**: テキスト & ポートレートUI (完了)。
- **コアシステム**:
    - **`GlobalState`**: グローバルフラグ・変数管理 (例: "has_met_boss", "evidence_count")。
    - **`SceneTransitionManager`**: フェード付きシーン切り替え。
    - **`ChoiceManager`**: 選択肢表示と分岐処理。
    - **`FlagCondition`**: フラグベースの条件分岐。
- **証拠品システム**:
    - **`EvidenceItem`**: 証拠品データ (名前、説明、アイコン、カテゴリ)。
    - **`EvidenceManager`**: 証拠品の追加/削除/確認。
    - **`InventoryUI`**: 証拠品リスト表示と詳細ビュー。
    - **`EvidencePresentSystem`**: ダイアログ中に証拠品を提示。
- **キャラクター表現**:
    - **`CharacterPortrait`**: 複数表情/ポーズのスプライト。
    - **`PortraitManager`**: ダイアログに応じた立ち絵切り替え。
    - **`TextAnimator`**: タイプライター効果。
- **調査モード**:
    - **`InvestigationScene`**: ホットスポットをクリックして調べる/話す/移動。
    - **`HotspotManager`**: クリック可能エリアと説明の定義。
    - **`LocationMap`**: シーン移動用のビジュアルマップ。
- **法廷/対決モード**:
    - **`TestimonySystem`**: 証言を1行ずつ表示。
    - **`ContradictionDetector`**: ゆさぶる/つきつける機能。
    - **`HealthBar`**: 誤答ペナルティシステム。
- **デモシーン**:
    - **調査シーン**: オフィスで手がかりとNPCを探す。
    - **会話シーン**: 選択肢付きの会話。
    - **証拠提示シーン**: 法廷スタイルの証拠品提示。
    - **マルチエンディング**: フラグベースのストーリー分岐。

#### 格闘ゲーム (Phase 6)
- **`FightingEntity`**:
    - 厳密なステートマシン (フレーム単位)。
    - Hitbox/Hurtboxシステム (物理衝突とは別)。
    - 必殺技用の入力バッファ。
- **ビュー**: `SideScrollingView` を使用するが、カメラ範囲は固定。

#### サンドボックス (Phase 7)
- **`MiningSystem`**:
    - `UniversalWorldData` と連携してボクセルを切り替え (壁 -> 床)。
    - ドロップアイテム。
- **`CraftingSystem`**:
    - レシピデータ構造。

#### シューティングゲーム (Phase 8)
- **`BulletManager`**:
    - 弾のオブジェクトプーリング。
    - 弾幕パターン生成器。
- **`EnemyWaveManager`**:
    - Wave単位での敵スポーン。

#### パズルゲーム (Phase 9)
- **`PuzzleBoard`**:
    - グリッドベースのタイルマッチング。
    - パターン認識 (match-3など)。
- **`FallingBlockSystem`**:
    - 重力ベースのタイル落下。

#### ストラテジー (Phase 10)
- **`RTSUnitController`**:
    - 複数ユニット選択 & コマンド。
    - パスファインディング (A* / Flow field)。
- **`TurnManager`**:
    - イニシアチブベースのターン順。

#### カードゲーム (Phase 11)
- **`Card`、`Deck`、`Hand`**:
    - カードデータ、デッキシャッフル、手札管理。
- **`CardBattleManager`**:
    - カードバトルのターンフロー。

#### タワーディフェンス (Phase 12)
- **`Tower`、`TowerPlacement`**:
    - タレット配置と射程管理。
- **`WaveSpawner`、`PathManager`**:
    - Wave生成と経路管理。

#### レーシング (Phase 13)
- **`VehicleController`**:
    - 加速、ステアリング、ドリフト。
- **`TrackManager`**:
    - チェックポイント & ラップシステム。

#### シミュレーション (Phase 14)
- **`ResourcePool`、`ProductionChain`**:
    - 複雑な経済システムとリソース変換。
- **`NPCScheduler`、`RelationshipSystem`**:
    - NPC日課と関係性管理。

#### メトロイドヴァニア (Phase 15)
- **`MapRevealSystem`**:
    - フォグオブウォーとエリア発見。
- **`AbilityGate`、`PowerupManager`**:
    - 能力ゲートと収集可能なアビリティ。

## 移行ステップ

1-10. **リファクタリング & コア実装** (完了)
11. **サンプルデモの実装**: 
    - 5つのデモタイプすべてのプレースホルダーを作成。
12. **セットアップスクリプトの実装**: 新しいタイプ用に `setup_demo.sh` を更新。
13. **機能の段階的実装**:
    - ユーザーの選択に基づいて優先順位を決定 (現在アドベンチャー?)。

## 検証
- **ビルド**: `scons target=template_debug`
- **実行**:
    - `./scripts/setup_demo.sh [type]` で各メインシーンをテスト。
