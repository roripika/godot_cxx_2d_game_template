# システムデータ設計 (System Data Design)

## 1. クラス構造 (Class Architecture)

### KarakuriScenarioRunner (C++)
- **役割**: YAMLシナリオのパース、アクション実行、およびホットスポットの動的管理。
- **主要な属性**:
    - `hotspot_bindings_`: ノードとシナリオアクションの紐付け。
    - `waiting_for_dialogue_`: ダイアログ終了待ち状態。
- **主要な機能**:
    - **初期非表示制御**: シーンロード時、`hs_*` プレフィックスを持つ全ての `Area2D` ノードを自動的に非表示にし、プロセスを無効化。
    - **動的配置**: YAMLの定義に基づき、ノードの可視状態 (`visible`)、位置 (`position`)、テクスチャ (`texture`) を動的に更新。

### InteractionManager (C++)
- **役割**: グローバルなクリック位置の検知と、世界への信号送信。
- **仕様**: `_unhandled_input` により、UIで処理されなかったイベントのみを `clicked_at` として発火。

### AdventureGameState (C++)
- **役割**: ゲーム内フラグ（健康状態、進行度、アイテム所持状況）の永続管理と提供。

## 2. リソース定義 (Data Resources)

### 共通アイテム定義 (items.yaml)
- **役割**: テンプレートで作成するゲーム全般で用いる「アイテム・証拠品」のビジュアル・固定順序の一元管理。
- **データ構造**:
  ```yaml
  items:
    item_id:
      sort_order: 10
      name_key: "translation_key_for_name"
      desc_key: "translation_key_for_description"
      icon: "res://path/to/icon.png"
  ```
- **運用**: 
  - UIでは、このYAMLからデータを読み込み、辞書型のマスターデータあるいは内部クラス (`EvidenceItem` 等) として保持・活用します。
  - ゲーム固有のフラグや用途設定が必要な場合は、これとは別のゲームモード別YAML等で、この `item_id` への参照をキーとして定義します。
  - アイテム一覧の表示順は常にこの `sort_order` の昇順で表示されます。

### シナリオ定義 (mystery.yaml)
- **構成**: `scenes` (場所), `actions` (共通動作), `variables` (全般)。
- **シーン定義の拡張**:
    - `hotspots`: シーンごとのホットスポット設定。
        - `node_id`: Godot上のノード名。
        - `visible`: (bool) 初期表示状態。
        - `position`: ([x, y]) 座標のオフセット。
        - `texture`: (path) スプライトテクスチャのパス（子ノードの `Sprite2D` に適用）。
- **アクション種別**: `dialogue`, `give_evidence`, `if_flag`, `goto`, `transition_object` 等。

## 3. 局所データ構造

### HotspotBinding (Struct)
- `hotspot_id`: YAML側での名前。
- `node_id`: Godot上のノードパス。
- `on_click`: 実行されるアクションの配列。
