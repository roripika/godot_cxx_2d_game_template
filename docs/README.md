# Karakuri Project Documentation Index

このディレクトリは、Karakuri Game OS Kernel v2.0 の設計、各フィットネステストの仕様、および監査レポートを管理しています。

## 📁 フォルダ構成

### 1. `docs/` (Root) - 現在の仕様と到達点
プロジェクトの現在の状態や、各モジュールの設計仕様が置かれています。
- [phase2_summary.md](phase2_summary.md): **Phase 2 (Infrastructure) の総括と到達点。**
- [infrastructure_backlog.md](infrastructure_backlog.md): 基盤フェーズで特定された残課題・将来課題。
- [roguelike_test_design.md](roguelike_test_design.md): ローグライク・フィットネステストの設計。
- [rhythm_test_design.md](rhythm_test_design.md): リズム・フィットネステストの設計意図と仕様。
- [rhythm_test_completion.md](rhythm_test_completion.md): リズムテストの検証結果と到達点。
- [kernel_test_matrix.md](kernel_test_matrix.md): 全モジュールのカバレッジ・マトリクス。

### 2. `docs/design/` - 将来設計と深掘り
次フェーズ以降で導入予定の機能や、アーキテクチャの深い論理設計を扱います。
- [real_input_bridge.md](design/real_input_bridge.md): 非同期な実入力と決定論カーネルの境界設計。
- [stress_test_strategy.md](design/stress_test_strategy.md): 大規模・高負荷検証のためのロードマップ。

### 3. `docs/reports/` - 監査・検証レポート
コードレビューや設計監査の結果を正式な記録として保管しています。
- [phase2_audit_report.md](reports/phase2_audit_report.md): 基盤フェーズ全体の差分監査。
- [roguelike_audit_report.md](reports/roguelike_audit_report.md): `roguelike_test` の個別監査。
- [rhythm_audit_report.md](reports/rhythm_audit_report.md): `rhythm_test` の個別監査。

---
> **Note to AI Agents**:
> 新しい機能の設計や Task の追加を行う前に、必ず `phase2_summary.md` と `kernel_test_matrix.md` を読み、既存の Kernel 契約（Core 非変更の原則など）を遵守してください。
