# システムデータ設計 (System Data Design)

## 1. クラス構造 (Class Architecture)

### KarakuriScenarioRunner (C++)
- **役割**: YAMLシナリオのパース、アクション実行、およびホットスポットの管理。
- **主要な属性**:
    - `hotspot_bindings_`: ノードとシナリオアクションの紐付け。
    - `waiting_for_dialogue_`: ダイアログ終了待ち状態。
    - `evidence_ui_`: インベントリUIノードへの参照。

### InteractionManager (C++)
- **役割**: グローバルなクリック位置の検知と、世界への信号送信。
- **仕様**: `_unhandled_input` により、UIで処理されなかったイベントのみを `clicked_at` として発火。

### AdventureGameState (C++)
- **役割**: ゲーム内フラグ（健康状態、進行度、アイテム所持状況）の永続管理と提供。

## 2. リソース定義 (Data Resources)

### EvidenceItem (.tres)
- **型**: カスタムリソース。
- **プロパティ**:
    - `item_id`: ユニークな内部識別子。
    - `item_name_key`: 翻訳キー。
    - `description_key`: 翻訳キー。
    - `icon_texture`: UI表示用のテクスチャ。

### シナリオ定義 (mystery.yaml)
- **構成**: `scenes` (場所), `actions` (共通動作), `variables` (全般)。
- **アクション種別**: `dialogue`, `give_evidence`, `if_flag`, `goto`, `transition_screen` 等。

## 3. 局所データ構造

### HotspotBinding (Struct)
- `hotspot_id`: YAML側での名前。
- `node_id`: Godot上のノードパス。
- `on_click`: 実行されるアクションの配列。
