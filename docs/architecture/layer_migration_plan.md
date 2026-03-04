# Layered Architecture Migration Plan (Karakuri vs Mystery)

## 1. Objective (目的)
現在の `src/` ディレクトリは「汎用的なゲーム基盤（Karakuri）」と「ミステリーゲーム特有のロジック（Mystery）」が混在しており、別ジャンルのゲームや次回作へテンプレートとして流用しづらい構造になっています。

本計画書は、M4 MacBook Airの強力なビルド環境を活かし、プロジェクト全体を明確な **2層構造（Layered Architecture）** に分離・再定義し、流用性の高いSDK基盤と、その上に乗るアプリケーション層をクリーンに分割するための設計書です。

## 2. Architecture Principles (階層と制約)

### Layer 1: Basic Game Karakuri (`src/core/`)
- **役割**: あらゆるジャンルの2Dゲームで再利用可能な「ゲーム基盤（エンジン拡張）」。
- **Namespace**: `karakuri::`
- **制約**: **絶対に上位層（Template層）の知識を持ってはならない。** `#include` において `src/mystery/` 以下のファイルを参照することは禁止される。

### Layer 2: Game Template (`src/mystery/`)
- **役割**: ミステリー・アドベンチャーゲームを成立させるための具体的なシステム実装。
- **Namespace**: `mystery::`
- **制約**: `karakuri::` 層が提供する汎用インターフェースを利用して構築する。

---

## 3. Migration Map (ファイル再配置計画)

現在の全C++ファイルを検証し、以下の配置に整理します。

### 🔄 Layer 1 へ移行/統合されるファイル群（`src/core/` への集約）

現状 `src/karakuri/` や `src/entities/` などに散らばっている基盤系コードを `src/core/` に統合します。
（※物理ディレクトリも `src/core/` に移動・統一します）

**【基盤・システムユーティリティ】**
- `src/karakuri/karakuri.h` -> `src/core/karakuri.h`
- `src/karakuri/karakuri_logger.*` -> `src/core/logger/karakuri_logger.*`
- `src/karakuri/karakuri_save_service.*` -> `src/core/services/save_service.*`
- `src/karakuri/karakuri_localization_service.*` -> `src/core/services/localization_service.*`
- `src/karakuri/yaml/karakuri_yaml_lite.*` -> `src/core/yaml/yaml_lite.*`

**【汎用エンティティ・コンポーネント】**
現在 `src/entities/` や `src/items/` にあるものは、ミステリー等の特定ジャンルに依存しない限り Core に属します。
- `src/entities/game_entity.*` -> `src/core/entities/base_entity.*` (名称変更・汎用化)
- `src/entities/components/hitbox_component.*` -> `src/core/components/hitbox_component.*`
- `src/entities/components/hurtbox_component.*` -> `src/core/components/hurtbox_component.*`
- `src/entities/components/raycast_component.*` -> `src/core/components/raycast_component.*`
- `src/items/game_item.*` -> `src/core/items/base_item.*`
- `src/items/inventory.*` -> `src/core/items/inventory_system.*`

**【シナリオ基盤（YAMLパーサー・ランナー）】**
シナリオを流すエンジン部分は Core に属します（※ただし、証言や対決など具体的なコマンド処理は除外）。
- `src/karakuri/scenario/karakuri_scenario_runner.*` -> `src/core/scenario/scenario_runner.*`

---

### 🧩 Layer 2 へ移行されるファイル群（`src/mystery/` への集約）

機能として実装されていたミステリー特有のシステムを `src/mystery/` に隔離します。

**【ミステリーシステム中核】**
- `src/features/mystery/mystery_manager.*` -> `src/mystery/mystery_manager.*`
- `src/features/mystery/enums.hpp` -> `src/mystery/enums.hpp`
- `src/features/mystery/evidence_manager.*` -> `src/mystery/evidence_manager.*`

**【ミステリー特有の拡張・具象クラス】**
- `src/karakuri/scenario/karakuri_testimony_session.h` -> `src/mystery/scenario/testimony_session.h` (※ CoreのRunnerに依存注入する設計へ変更)
- *(新規作成予定)* `src/mystery/entities/mystery_character.*` : Core層の `BaseEntity` を継承し、証言フラグや好感度などミステリー特有の変数を持つ具象クラス。

---

### 🗑️ 削除または別レイヤーへ隔離するファイル群

ミステリーに関連しない、実験的または別ジャンル向けのコード群は、プロジェクトの「ミステリーテンプレート化」という目的に対してノイズとなるため、`src/plugins/` などに隔離するか削除を提案します。

- `src/features/sandbox/` (マイニング等)
- `src/features/rhythm/` (リズムゲーム)
- `src/entities/fighter_controller.*` (格闘)
- `src/entities/player_controller_iso.*` (見下ろし)
- `src/views/isometric_view.*`

---

## 4. Logic Flow & Boundaries (境界の依存解決)

最も重要なのは **「Core層の ScenarioRunner が、どうやって Mystery層の TestimonySession (証言システム) を実行するか？」** という依存の逆転です。

```mermaid
graph TD
    subgraph Layer 1: Basic Game Karakuri (Core)
        CR[ScenarioRunner]
        AE[ActionExecutor Interface]
        CR --> AE
    end

    subgraph Layer 2: Game Template (Mystery)
        MM[MysteryManager]
        TS[TestimonyActionExecutor]
        MC[MysteryCharacter]
        
        TS -.-> |Implements| AE
        MM --> TS
        MM --> MC
    end
    
    CR -.-> |Calls registered action| TS
```

**解決策 (Action Dispatcherの実装)**:
Core側の `ScenarioRunner` は、ミステリーの「ゆさぶる (`shake`)」や「つきつける (`present`)」という具象コマンドを一切知るべきではありません。
代わりに、C++側で `register_custom_action("shake", std::function<void()>)` のようなインターフェースを Core に用意し、ゲーム起動時に `MysteryManager` がミステリー専用のアクション一覧を Core に「登録（注入）」する仕組み（DI: 依存性の注入）を採用します。

## 5. Quality Assurance (品質保証と自動チェック)

- **物理的な防波堤**: `SConstruct` ファイルを修正し、`src/core/` フォルダ内のコンパイル時における Include パス (`CPPPATH`) から意図的に `src/mystery/` を除外します。これにより、Core が Mystery に依存した瞬間に **コンパイルエラー** が発生し、構造破壊を物理的に防止します。
- **名前空間チェック**: `karakuri::` と `mystery::` の混在（`mystery::` が `src/core/` 内に現れるなど）を正規表現で検知する簡単なLintスクリプトを導入し、「意図しない依存」をCIでブロックします。
