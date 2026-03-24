# AI coding instructions for Karakuri Project

このプロジェクトは「AI が安全にゲームロジックを書ける Game OS Kernel」の構築を目的としています。
AI エージェントは以下の原則を厳守してください。

## 1. 開発の絶対原則
- **src/core 改変厳禁**: Kernel (GDExtension) コアロジックは凍結されています。いかなる理由があっても `src/core/` 以下のコードを変更しないでください。
- **Task 中心設計**: ロジックの拡張は `src/games/` 配下の `Task` クラスの新規作成と、YAML シナリオの記述のみで行ってください。
- **Fail-Fast**: 新しい Task を追加する際は、必ず `validate_and_setup()` でペイロードの厳格な検証を行ってください。

## 2. ドキュメント参照ルール（ノイズ抑制）
AI エージェントは、コンテキスト窓の Attention を最適化するため、以下の層分けを意識してください。
- **必読 (Layer A)**: `ARCHITECTURE.md`, `docs/kernel_test_matrix.md`, `docs/README.md`。
- **作業時のみ (Layer B)**: 編集中のファイルに関連する `docs/modules/*.md` や `docs/design/*.md`。
- **無視推奨 (Layer C)**: `docs/reports/` 配下の監査レポートは、技術的負債の調査時以外は読み込まないでください。

## 3. 既知の制約
- **pos-0 skip**: シーン遷移直後の最初のタスクがスキップされる不具合があります。YAML シナリオでは pos-0 に「犠牲スロット（同一タスクの複製など）」を置いて回避してください。詳細は `docs/infrastructure_backlog.md` を参照。
