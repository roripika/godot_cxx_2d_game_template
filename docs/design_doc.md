# Universal Game Template Design Document

## 概要 (Overview)
このドキュメントは、以下の3つの軸を組み合わせ可能な「ユニバーサル・ゲームテンプレート」のアーキテクチャ設計案です。

1.  **View (視点)**: 2D横/2Dクオーター/3D横/3Dクオーター/FPS
2.  **Style (遊び方)**: RPG/アクション/アドベンチャー/ノベル
3.  **Cycle (進行)**: ローグライク/ストーリー/サンドボックス

既存のGodot GDExtensionプロジェクトを拡張し、**C++による「コアシステム（データ・ロジック）」**と、**Godotシーンによる「ビュー（表現・操作）」**を完全に分離することでこれを実現します。

---

## アーキテクチャ構成

### 1. Core Layer (C++ GDExtension)
ゲームの「状態」と「計算」を担当します。視点やジャンルに依存しない汎用的なクラス群です。

*   **`UniversalWorld` (Data Model)**
    *   現在の `SandboxWorld` を抽象化。
    *   3次元配列 (x, y, z) として世界データを保持。
    *   2Dゲームの場合は z=0 のみ使用、あるいはレイヤーとして使用。
    *   **機能**: チャンク管理、保存/読み込み、PCG (Procedural Content Generation) インターフェース。
*   **`GameCycleManager`**
    *   ゲームループのステートマシン。
    *   `RoguelikeSession`: ランダム生成 -> プレイ -> リザルト -> リセット
    *   `StorySession`: フラグ管理、シーン遷移管理
    *   `SandboxSession`: 永続的なワールドストリーミング
*   **`EntityManager`**
    *   プレイヤー、敵、NPCの基本ステータス管理 (RPG/Action共通)。
    *   インベントリシステム。

### 2. View Layer (Godot Scenes)
C++のデータをどのように「見せる」かを担当します。各視点ごとに専用のシーン/プレハブを用意します。

*   **Controllers (Player Inputs)**
    *   `PlayerController2D_Side`: 物理(Platformer)挙動。
    *   `PlayerController2D_Iso`: グリッドベース/自由移動。
    *   `PlayerController3D_FPS`: キャラクターコントローラー + カメラ操作。
    *   これらは共通の `EntityManager` にコマンド ("Move", "Attack") を送ります。
*   **WorldRenderers**
    *   **Iso2D**: 現在の `TileMapLayer` 実装を使用。 `UniversalWorld` のデータを読み取りタイルを配置。
    *   **Side2D**: `UniversalWorld` の垂直断面 (x, y) をタイルとして描画。
    *   **Iso3D**: `GridMap` または `MultiMeshInstance3D` を使用してボクセル/メッシュとして描画。
*   **NovelSystem**
    *   GodotのUI (Controlノード) ベースの会話エンジン。どの視点の上にもオーバーレイ可能。

---

## 実装ロードマップ案

### Phase 1: Core Refactoring (汎用化)
現在の `SandboxWorld` (Isometric Sandbox専用) を、より汎用的なデータコンテナ `WorldData` と、それを描画する `IsometricTileMapLayer` に分離する。

### Phase 2: View Adapters (視点追加)
同じ `WorldData` を入力として、以下を表示できるデモを作成する。
1.  **Isometric View** (既存)
2.  **Side-Scrolling View** (データを横から見た図として解釈)
3.  **Experimental 3D View** (データを3Dメッシュとしてインスタンス化)

### Phase 3: Game Cycles (サイクル実装)
`setup.sh` または Godotエディタ上の設定で、「ローグライクモード」や「ストーリーモード」を切り替えられる `GameManager` シングルトンを実装する。

## ディレクトリ構成イメージ

```
src/
├── core/
│   ├── world_data.cpp       # 汎用データ (3次元配列/ハッシュマップ)
│   ├── game_state.cpp       # サイクル管理
│   └── entity_base.cpp      # RPG/Action共通ステータス
├── views/
│   ├── iso_tile_renderer.cpp # 2D Iso用 (TileMapLayer継承)
│   ├── voxel_renderer.cpp    # 3D用 (GridMap/Mesh継承)
│   └── side_renderer.cpp     # 2D Side用
```
