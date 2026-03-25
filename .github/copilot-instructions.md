# AI coding instructions for Karakuri Project

このプロジェクトは「AI が安全にゲームロジックを書ける Game OS Kernel」の構築を目的としています。
AI エージェントは以下の原則を厳守してください。

## 1. 開発の絶対原則
- **src/core 改変厳禁**: Kernel (GDExtension) コアロジックは凍結されています。いかなる理由があっても `src/core/` 以下のコードを変更しないでください。
- **Task 中心設計**: ロジックの拡張は `src/games/` 配下の `Task` クラスの新規作成と、YAML シナリオの記述のみで行ってください。
- **Fail-Fast**: 新しい Task を追加する際は、必ず `validate_and_setup()` でペイロードの厳格な検証を行ってください。

## 2. docs 参照ルール（ノイズ抑制）

**まず `docs/README.md` を読む。** これが docs 全体のゲートウェイです。

### 層A — 常時参照（3本のみ）
- `ARCHITECTURE.md` — Kernel 基本原則
- `docs/kernel_test_matrix.md` — モジュールカバレッジ地図
- `docs/README.md` — docs ゲートウェイ

### 層B — 作業時のみ参照
編集中のモジュールに関連する `docs/<module>_design.md` や `docs/<module>_completion.md`。

| モジュール | 読むファイル |
| :--- | :--- |
| mystery_test | `docs/mystery_design.md` |
| roguelike_test | `docs/roguelike_test_design.md` / `docs/roguelike_test_completion.md` |
| rhythm_test | `docs/rhythm_test_design.md` / `docs/rhythm_test_completion.md` |

フェーズ移行時は `docs/phase2_summary.md` や `docs/infrastructure_backlog.md` を追加参照。

### 層C — 原則無視
- **`docs/reports/`** 配下の監査レポートは、技術的負債の調査時以外は読み込まないでください。

## 3. 既知の制約
- **pos-0 skip**: シーン遷移直後の最初のタスクがスキップされる不具合があります。YAML シナリオでは pos-0 に「ܮܮスロット（同一タスクの複製など）」を置いて回避してください。詳細は `docs/infrastructure_backlog.md` を参照。
