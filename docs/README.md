# Karakuri Project Documentation Guide (AI & Human Gateway)

このディレクトリは、Karakuri Game OS Kernel v2.0 の設計、各フィットネステストの仕様、および監査レポートを管理しています。
**AI エージェント（Copilot/Antigravity）は、まずこのガイドを確認し、作業に必要な最小限の文書のみをコンテキストに読み込んでください。**

---

## 🚦 AI 読解ガイド (Attention Control)

AI エージェントは、コンテキスト窓のノイズを減らすため、以下の優先度に従ってドキュメントを参照してください。

### 1. 層A: 基盤契約 (常時参照・絶対遵守)
**新しい Task の作成や既存ロジックの変更時には、常に以下の 3 ファイルを基盤コンテキストとして維持してください。**
- **[ARCHITECTURE.md](../ARCHITECTURE.md)**: Kernel v2.0 の基本原則（Core 改変厳禁、Task 拡張）。
- **[kernel_test_matrix.md](kernel_test_matrix.md)**: 全モジュールの現状とカバレッジ範囲の地図。
- **この README.md**: 構造理解のためのエントリポイント。

### 2. 層B: 設計仕様 (作業時のみ参照)
**特定のモジュールや機能に触れる場合、またはフェーズの移行時のみ参照してください。**
- **[phase2_summary.md](phase2_summary.md)**: 基盤フェーズの到達点。新フェーズ開始時に参照。
- **[infrastructure_backlog.md](infrastructure_backlog.md)**: 既知の制限（pos-0 skip 問題など）や回避策の記録。
- **各モジュール設計書**:
    - [roguelike_test_design.md](roguelike_test_design.md) / [rhythm_test_design.md](rhythm_test_design.md)
- **将来設計メモ (`docs/design/`)**:
    - [real_input_bridge.md](design/real_input_bridge.md) / [stress_test_strategy.md](design/stress_test_strategy.md)

### 3. 層C: 記録資料 (原則無視)
**過去のデバッグや監査の経緯を調べる場合を除き、AI はこれらを読み込まないでください。**
- **監査レポート (`docs/reports/`)**: 差分監査や個別モジュールの監査記録。
- **[walkthrough.md](walkthrough.md)**: 開発過程のプログレスログ。

---

## 📁 ディレクトリ構造の役割

| フォルダ | 役割 | AI への推奨 |
| :--- | :--- | :--- |
| `docs/` | 現在の構成・設計・完了メモ | 層A/B を含む。必要に応じて抽出。 |
| `docs/design/` | 基盤外の将来的な設計（未実装） | 特定の将来検討タスク以外は無視。 |
| `docs/reports/` | 人間向けの監査・検証記録 | **原則無視。** ノイズ回避。 |

---
> [!IMPORTANT]
> **AI エージェントへの最重要命令**:
> `src/core/` 以下のファイルは Kernel の聖域です。実装の拡張は必ず `src/games/` 配下の Task クラスと YAML シナリオでのみ行ってください。
