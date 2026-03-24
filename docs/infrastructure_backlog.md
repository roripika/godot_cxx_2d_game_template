# Infrastructure Backlog (Phase 2 未解決課題)

## 1. 概要
本ドキュメントは、基盤フェーズ（Phase 2）において特定されたものの、カバレッジ達成のブロッカーではないため次フェーズ以降に回された課題を管理します。

## 2. カテゴリ別課題

### 2.1 GDExtension / エンジン連携
- **ActionRegistry 初期化順序**: 現在 `initialize_module` 内で手動生成しているが、Godot の Autoload との競合をより堅牢に（Engine singleton 等で）管理する仕組みの検討。
- **GDExtension 再ロード耐性**: エディタ上での Hot-reload 時に WorldState が正しく破棄・再生成されないケースの調査。

### 2.2 データ形式 (YAML / Parser)
- **YamlLite の制限**: 現在の簡易パーサでは多重ネストやアンカー・エイリアスが使えない。大規模シナリオに向けて、フル機能の YAML パーサへの移行を検討。
- **スキーマ定義の厳格化**: `validate_scenario.py` を JSON Schema 等の標準的な形式に移行し、エディタ上でのリアルタイムバリデーションを可能にする。

### 2.3 ScenarioRunner / Task 実行
- **Task の並列実行 (ParallelTask)**: 現在の `parallel` アクションは時分割擬似並列だが、CPU コアを活かした真の並列実行が必要かどうかの再判定。
- **分岐の可視化**: 複雑な `if_flag` や `check_condition` のネストを、グラフ図として自動出力する機能。

### 2.4 WorldState
- **大規模データの永続化**: 1,000件を超えるエビデンスやアイテムがある場合のセーブ・ロード時間の最適化。
- **変更通知 (Reactive)**: WorldState の特定キーが変化した際に、Task や UI に通知を送る仕組み（現在は毎フレーム監視が必要）。

## 3. 次フェーズへの持ち越し判断
上記課題は、**「AIが安全にゲームロジックを書ける」という最小要件**を阻害するものではありません。したがって、Phase 2 の完了を妨げるものではなく、Phase 3 (Template/Assist) 以降の最適化フェーズで順次対応予定です。
