# ユニバーサル・ゲームテンプレート ウォークスルー

## 概要
このプロジェクトは、コアロジックとビューを分離し、同じデータを使ってアイソメトリックRPGと横スクロールプラットフォーマーの両方を実現します。

## 主要コンポーネント

### 1. ビュー層 (`src/views`)
- **`IsometricView`**: トップダウン/アイソメトリック表示。
- **`SideScrollingView`**: サイドビュー (プラットフォーマー) 表示。

### 2. エンティティ層 (`src/entities`)
- **`PlayerControllerIso`**: アイソメトリック操作。
- **`PlayerControllerSide`**: プラットフォーマー操作。

### 3. ループ & サイクル (`src/core/cycles`)
- **`RoguelikeGenerator`**: マップデータを生成するアルゴリズム。
- **`RoguelikeManager`**: レベル生成とプレイヤースポーンを統括するノード。

### 4. アドベンチャー機能 (`src/core`)
- **`InteractionManager`**: マウスインタラクションを処理。
    - シグナル: `clicked_at(position)`
    - 座標空間: `position` は Canvas/World 座標（`Area2D.global_position` と同じ空間）。
    - 使い方: `active` プロパティでインタラクションの有効/無効を切り替え。
- **`DialogueUI`**: 名前ラベル付きのテキストメッセージを表示。
    - 関数: `show_message(name, text)`、`hide_dialogue()`。
    - 使い方: シーンに追加し、スクリプトから `show_message` を呼び出す。

## サンプルシステム

このプロジェクトには、異なるゲームテンプレートを簡単に切り替えられるデモシステムが含まれています。

### レイアウト
- **`samples/roguelike/`**: ローグライクデモ。
- **`samples/platformer/`**: 横スクロールデモ。
- **`samples/mystery/`**: アドベンチャーゲームデモ。
- **`samples/sandbox/`**: 採掘 & クラフトデモ。
- **`samples/fighting/`**: 格闘ゲームデモ。

### デモの切り替え
提供されているスクリプトを使ってプロジェクトの起動シーンを切り替えます。

```bash
# ローグライクデモに切り替え
./scripts/setup_demo.sh roguelike

# プラットフォーマーデモに切り替え
./scripts/setup_demo.sh platformer

# ミステリーデモに切り替え
./scripts/setup_demo.sh mystery

# サンドボックスデモに切り替え
./scripts/setup_demo.sh sandbox

# 格闘デモに切り替え
./scripts/setup_demo.sh fighting
```

## 使い方ガイド: ローグライクデモの作成

手動でプレイ可能なデモシーンを作成するには:

1. ルートノードに **`PlayerControllerIso`** を持つ新しいシーンを作成。
2. `Sprite2D` と `CollisionShape2D` を追加。
3. `res://player.tscn` として保存。
4. `RoguelikeManager` と `IsometricView` を持つ新しいレベルシーンを作成。
5. インスペクターでリンクして実行。
