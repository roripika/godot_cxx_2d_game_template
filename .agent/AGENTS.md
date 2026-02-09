# AI Agent Guidelines

このプロジェクトにおけるAIアシスタントの行動指針とルールです。

## 1. 言語設定 (Language)
*   **やりとり**: 基本的に **日本語** で行います。
*   **ドキュメント**: 設計書、README、コミットメッセージ等は日本語で記述します。
*   **コードコメント**: 重要なロジックの説明は日本語で行うことを推奨します。

## 2. プロジェクトアーキテクチャ (Architecture)
このプロジェクトは **Universal Game Template** (モジュラー型ゲームテンプレート) です。以下の設計思想を維持してください。

*   **Core / View の分離 (Separation of Concerns)**:
    *   `src/core`: ゲームロジックとデータのみを実装します。Godotの描画APIやシーングラフへの直接依存を避けてください。
    *   `src/views`: `Core`のデータを可視化・操作するための「アダプター」です。ゲームの主要ロジックをここに書かないでください。

*   **互換性**:
    *   **Godot Version**: 4.3 以上 (`TileMapLayer` 等を使用するため)
    *   **Platform**: macOS (Apple Silicon) を主要ターゲットとしつつ、クロスプラットフォームを意識したコードを書くこと。

## 3. ファイル構成
*   `src/core`: `UniversalWorldData`, `GameCycleManager` など
*   `src/views`: `IsometricView`, `SideScrollingView` など
*   `src/entities`: `GameEntity` など
